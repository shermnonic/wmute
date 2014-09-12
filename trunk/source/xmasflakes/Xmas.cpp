/*****************************************************************************
 *
 * Christmas Exercise (snowfield attractor)
 * Lecture "Introduction to Computer Graphics", Prof. Re.Klein
 *
 * Functions to implement:
 * -----------------------
 *  - This code here is fine. You have to fix the fragment shader 
 *    in updateFlakes.frag!
 *
 * TODO (not for students):
 * - Translate german comments in display() function.
 *
 * (c)2007-2011 Institute of Computer Science II, University of Bonn
 *
 *****************************************************************************/
#include <iostream>
#include <fstream>
#include <time.h>

#ifdef _WIN32
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#endif

#include "Trackball.h"

#define FILE_WINDFIELD         "data/3rdparty/windfield.dat"
#define FILE_UPDATEFLAKES_VERT "updateFlakes.vert"
#define FILE_UPDATEFLAKES_FRAG "updateFlakes.frag"


using namespace std;


SimpleTrackball trackBall; // the trackball object
GLuint framebufferObject;  // a framebuffer object to render snow flake positions offscreen
GLuint updateFlakePositionProg; // vertex and fragment programs to update snow flake positions
const size_t numberOfFlakesExp = 18; // create 2^18 snow flakes 
GLuint flakePositionTex[2]; // texture that holds snow flake positions on the GPU 
                            // (we have two. in each pass we read from one and write in the other.
                            // after the rendering pass we flip the textures)
GLuint windFieldTex;        // 3d texture that holds the wind field
const unsigned int windFieldSize = 128; // size of the wind field texture
GLuint positionUniform;     // positionSampler uniform for frag shader
GLuint windSpeedUniform;    // windspeedSampler uniform for frag shader
GLuint vertexVBO;           // a vertex buffer object so we can use the updated flake positions on the GPU
                            // as vertices in the rendering  pass.



GLint getUniLoc(GLuint program, const GLchar *name)
{
  GLint loc;

  loc = glGetUniformLocation(program, name);

  if (loc == -1)
    printf("No such uniform named \"%s\"\n", name);

  return loc;
}



void printShaderInfoLog(GLuint shader)
{
  int infologLength = 0;
  int charsWritten  = 0;
  GLchar *infoLog;

  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);

  if (infologLength > 0)
    {
      infoLog = (GLchar *)malloc(infologLength);
      if (infoLog == NULL)
        {
	  printf("ERROR: Could not allocate InfoLog buffer\n");
	  exit(1);
        }
      glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
      printf("Shader InfoLog:\n%s\n\n", infoLog);
      free(infoLog);
    }
}



void printProgramInfoLog(GLuint program)
{
  int infologLength = 0;
  int charsWritten  = 0;
  GLchar *infoLog;

  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);

  if (infologLength > 0)
    {
      infoLog = (GLchar *)malloc(infologLength);
      if (infoLog == NULL)
        {
	  printf("ERROR: Could not allocate InfoLog buffer\n");
	  exit(1);
        }
      glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
      printf("Program InfoLog:\n%s\n\n", infoLog);
      free(infoLog);
    }
}



int installShaders(const GLchar *Vertex, const GLchar *Fragment,
		   GLuint *Prog)
{
  GLuint VS, FS;   // handles to objects
  GLint  vertCompiled, fragCompiled;    // status values
  GLint  linked;

  // Create a vertex shader object and a fragment shader object

  VS = glCreateShader(GL_VERTEX_SHADER);
  FS = glCreateShader(GL_FRAGMENT_SHADER);

  // Load source code strings into shaders

  glShaderSource(VS, 1, &Vertex, NULL);
  glShaderSource(FS, 1, &Fragment, NULL);

  // Compile the  vertex shader, and print out
  // the compiler log file.

  glCompileShader(VS);
  glGetShaderiv(VS, GL_COMPILE_STATUS, &vertCompiled);
  printShaderInfoLog(VS);

  // Compile the  vertex shader, and print out
  // the compiler log file.

  glCompileShader(FS);
  glGetShaderiv(FS, GL_COMPILE_STATUS, &fragCompiled);
  printShaderInfoLog(FS);

  if (!vertCompiled || !fragCompiled)
    return 0;

  // Create a program object and attach the two compiled shaders

  *Prog = glCreateProgram();
  glAttachShader(*Prog, VS);
  glAttachShader(*Prog, FS);

  // Link the program object and print out the info log

  glLinkProgram(*Prog);
  glGetProgramiv(*Prog, GL_LINK_STATUS, &linked);
  printProgramInfoLog(*Prog);

  if (!linked)
    return 0;

  return 1;
}


/**
 * generate random positions for snow flakes
 */
void initFlakes(float* positions){
  srand(time(0));
  for(unsigned int i=0;i<3*(1 << numberOfFlakesExp);i += 3){
    positions[i] = -50.f + 100.f*(float(rand()) / (float)RAND_MAX);
    positions[i+1] = -50.f + 100.f*(float(rand()) / (float)RAND_MAX);
    positions[i+2] = 100.f + 500.f*(float(rand()) / (float)RAND_MAX);
  }
}



/**
 * read windfield from file
 */
void readWindField(char* field){
  std::ifstream is;
  is.open(FILE_WINDFIELD, std::ios::binary);
  if(!is.good()){
    cout << "Could not open " << FILE_WINDFIELD << endl;
  }
  is.read(field, 3*windFieldSize*windFieldSize*windFieldSize*sizeof(char));
  is.close();
}



void createAndLoadTextures()
{
  // generate random flake positions
  float* positions;
  positions = new float[3*(1 << numberOfFlakesExp)];
  initFlakes(positions);
  
  // load field
  char* field;
  field = new char[3*windFieldSize*windFieldSize*windFieldSize];
  readWindField(field);


  // create textures for flake position
  glGenTextures(2, flakePositionTex);
  for(size_t i = 0; i < 2; ++i){
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, flakePositionTex[i]);
    // define texture with floating point format
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_FLOAT_RGBA32_NV, //GL_RGBA32F_ARB,
		 1 << (numberOfFlakesExp/2), 1 << (numberOfFlakesExp/2),
		 0, GL_RGB, GL_FLOAT, positions);
    // set texture parameters
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
		    GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
		    GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
		    GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
		    GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  

  // create and initialize 3D texture for vector field
  glGenTextures(1, &windFieldTex);
  glBindTexture(GL_TEXTURE_3D, windFieldTex);
  
  glTexImage3D(GL_TEXTURE_3D, 0, GL_SIGNED_RGB8_NV, //GL_RGBA32F_ARB,
	       windFieldSize, windFieldSize, windFieldSize,
	       0, GL_RGB, GL_BYTE, field);
  // set texture parameters
  glTexParameteri(GL_TEXTURE_3D,
		  GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D,
		  GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D,
		  GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_3D,
		  GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_3D,
		  GL_TEXTURE_WRAP_R, GL_CLAMP);

  
  delete[] positions;
  delete[] field;
}




void init(void)
{
  glClearColor(0, 0, 0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  gluPerspective(45, 1, 2, 1000);

  glShadeModel(GL_SMOOTH);	// select smooth shading

  // create a simple directional light source
  GLfloat mat_shininess[] = {4.f};
  GLfloat mat_specular[] = {1.f, 1.f, 1.f, 1.f};
  GLfloat mat_diffuse[] = {0.9f, 0.85f, 0.3f, 1.f};
  GLfloat mat_ambient[] = {0, 0, 0, 0};
  GLfloat light_white[] = {1, 1, 1, 1};
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
  GLfloat light_position0[] = {0.f, 0.f, 1.f, 0.f};
  GLfloat light_position1[] = {0.f, 0.f, 0.f, 1.f};
  glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_white);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_white);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_white);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
  glLightfv(GL_LIGHT1, GL_AMBIENT, light_white);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_white);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_white);

	
  //glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);

  // create and initialize FBO ( for offscreen rendering )
  glGenFramebuffersEXT(1, &framebufferObject);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebufferObject);
  createAndLoadTextures();
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  // create vertex buffer
  glGenBuffers(1, &vertexVBO);                 // Get A Valid Name
  glBindBuffer(GL_ARRAY_BUFFER, vertexVBO); // Bind The Buffer
  glBufferData(GL_ARRAY_BUFFER, 12*(1 << numberOfFlakesExp), 0, GL_DYNAMIC_DRAW);	
  glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind the buffer
}




void display(void)
{
  
  // erster Schritt: updaten der position der schneeflocken
  // dies passiert in einem offscreen frame buffer object (FBO). 
  // der color buffer dieses FBO wird dann später in ein vertex buffer object kopiert
  // das die positionen aller schneeflocken enthält. diese werden dann in einem zweiten schritt 
  // in einen on screen buffer gezeichnet.
  
  // vertex und fragment shader einschalten
  glUseProgram(updateFlakePositionProg);
  // off screen FBO auswählen
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebufferObject); // bind framebuffer object for render to texture
	
  // der color buffer dieses FBO wird eine der beiden flakePositionTex Texturen
  // in diese schreiben wir im ersten pass die upgedaten positionen.
  // wir brauchen zwei texturen, da beim rendern nicht in ein und dieselbe textur
  // geschrieben und gelesen werden kann.
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, 
  			    GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB,
  			    flakePositionTex[0], 0);
	
	
   
  // die zweite flakePositionTex textur benutzen wir normal als
  // input textur. weil wir meherer texturen brauchen kommt sie 
  // auf die textureinheit 1.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, flakePositionTex[1]);
  glEnable(GL_TEXTURE_RECTANGLE_ARB);
  glUniform1i(positionUniform, 0);

  // auf eine zweite textur einheit binden wir die 3d textur mit
  // dem windfeld
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_3D, windFieldTex);
  glEnable(GL_TEXTURE_3D);
  glUniform1i(windSpeedUniform, 1);
  glActiveTexture(GL_TEXTURE0);

  // wir zeichen nicht in den normalen on screen frame buffer, 
  // sondern in unser offscreen FBO
  glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
  
  // jetzt wird einfach ein quad mit allen schneeflocken gezeichnet
  // den rest macht der fragment shader
  // setup viewport to match flake texture proportions
  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glViewport(0, 0, 1 << (numberOfFlakesExp/2), 1 << (numberOfFlakesExp/2));
  
  // render screen sized quad (touch every pixel in the textures)
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glBegin(GL_QUADS );
  glTexCoord2f( 0, 0 );
  glVertex3i(-1, -1, 0);
		
  glTexCoord2f(1 << (numberOfFlakesExp/2), 0 );
  glVertex3i(1, -1, 0);

  glTexCoord2f(1 << (numberOfFlakesExp/2), 1 << (numberOfFlakesExp/2));
  glVertex3i(1, 1, 0);

  glTexCoord2f(0, 1 << (numberOfFlakesExp/2));
  glVertex3i(-1, 1, 0);
  glEnd();
  
  // vertex und frag shader ausschalten:
  glUseProgram(0);

  // jetzt kopieren wir die im shader gerenderten positionen in
  // ein Vertex Buffer Object. Dies ermöglicht es, die gerenderten
  // positionen im nächsten pass als vertices zu benutzen, ohne sie
  // von der grafikkarte in den hauptspeicher zu kopieren.
  glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, vertexVBO);
  glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  glReadPixels(0, 0, 1 << (numberOfFlakesExp/2), 1 << (numberOfFlakesExp/2), GL_RGB, GL_FLOAT, 0);
  glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
  
  // nicht vergessen die beiden positions texturen (input/output) zu vertauchen:
  std::swap(flakePositionTex[0], flakePositionTex[1]);
  
  // jetzt das FBO wieder ausschalten. 
  // wir zeichenen wieder ganz normal in den Back Buffer des on screen frame buffers
  // (nach dem glutSwapBuffers wird dieser ja zum Frontbuffer und damit angezeigt)
  // die texturen werden auch wieder abgeschaltet, weil sie im nächsten schritt sonst stören 
  // würden
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // unbind framebuffer object
  glDrawBuffer(GL_BACK);
  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_3D);
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_RECTANGLE_ARB);

  
  //
  // zweiter schritt: zeichnen der szene mit den im ersten schritt berechneten
  // schneeflocken positionen.
  //
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // setup matrices fpr scene drawing
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, 1, 2, 1000);

  glMatrixMode(GL_MODELVIEW);  // Setzt die Modelview Matrix als aktive Matrix
  glLoadIdentity();	     // Laed die Einheitsmatrix
  glTranslatef(0, 0, -100);
  trackBall.multModelMatrix(); // Binde Trackball ein
	
  
  // Ebene bei z = 0 auf der sich die Schneeflocken sammeln sollen:
  glColor3f(0.7f, 0.f, 0.3f);
  glBegin(GL_QUADS);
  glNormal3f(0,0,1);
  glVertex2i(-50,-50);
  glVertex2i(-50,50);
  glVertex2i(50,50);
  glVertex2i(50,-50);
  glEnd();

  // zeichnen der schneeflocken
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glPointSize(2.f);

  // jetzt weisen wir an, dass die positionen aus
  // dem VBO benutzt werden sollen
  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, vertexVBO); 
  glVertexPointer(3, GL_FLOAT, 0, 0); 

  glColor3f(0.1f, 0.5f, 0.1f);
  glDrawArrays(GL_POINTS, 0, 1 << numberOfFlakesExp);

  // VBO wieder ausschalten (sonst kann man im nächsten schritt nicht reinrendern)
  glDisableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glDisable(GL_BLEND);
  
  // Zeigt das gezeichnete Bild an
  glutSwapBuffers();
}




void mouseClick(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN)
    {
      switch(button)
	{
	case GLUT_LEFT_BUTTON:
	  trackBall.startBall(x, y);
	  break;
	case GLUT_MIDDLE_BUTTON:
	  trackBall.startZoom(x, y);
	  break;

	default:
	  break;
	}
    }
  if (state == GLUT_UP)
    {
      switch(button)
	{
	case GLUT_LEFT_BUTTON:
	  trackBall.stopBall(x, y);
	  break;
	case GLUT_MIDDLE_BUTTON:
	  trackBall.stopZoom(x, y);
	default:
	  break;
	}
    }
  glutPostRedisplay();
}




void mouseDrag(int x, int y)
{
  trackBall.rollBall(x, y);
  glutPostRedisplay();
}




GLchar *readFile(const char *filename)
{
  std::ifstream f;
  f.open(filename, std::ios::binary);
  if(!f.is_open() || !f.good())
    return 0;
  std::ifstream::pos_type begin_pos = f.tellg();
  f.seekg(0, std::ios_base::end);
  int fileSize = static_cast<int>(f.tellg() - begin_pos);
  f.seekg(0, std::ios_base::beg);
  GLchar *p = new GLchar[fileSize + 1];
  p[fileSize] = 0;
  f.read(p, fileSize);
  return p;
}




void idle()
{
  glutPostRedisplay();
}




int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(640, 640);
  glutCreateWindow("Christmas exercise");  
  glutSetWindowTitle("Christmas exercise");

#ifdef _WIN32
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if(GLEW_OK != err)
    {
      // Problem: glewInit failed, something is seriously wrong.
      std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
      return 0;
    }
  if(!GLEW_ARB_fragment_program || !GLEW_ARB_pixel_buffer_object
     || !GLEW_EXT_framebuffer_object
     || !GLEW_ARB_vertex_buffer_object)
    {
      std::cout << "Required OpenGL extensions are not supported on this system!" << std::endl;
      return 0;
    }
#endif


  // Initialize GL
  init();
  
  // read and install shader for updating snowflake positions
  GLchar *vertProgram = readFile(FILE_UPDATEFLAKES_VERT);
  GLchar *fragProgram = readFile(FILE_UPDATEFLAKES_FRAG);
  if(!fragProgram || !vertProgram)
    return 0;
  if(!installShaders(vertProgram, fragProgram, &updateFlakePositionProg))
    return 0;
  positionUniform = getUniLoc(updateFlakePositionProg, "FlakePositions");
  windSpeedUniform = getUniLoc(updateFlakePositionProg, "WindSpeed");

  glutMouseFunc(mouseClick);
  glutMotionFunc(mouseDrag);
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutMainLoop();


  return 0;
}
