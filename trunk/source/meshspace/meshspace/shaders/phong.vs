varying vec3 N;
varying vec3 v;
varying vec3 lightPos;
varying vec4 color;

void main(void)
{ 
	// v needed in fragment shader for lighting calculation.
	// Normalize and multiplication by NormalMatrix are required to achieve
	// equivalent results to OpenGL fixed pipeline.
	v = vec3(gl_ModelViewMatrix * gl_Vertex);   
	N = normalize(gl_NormalMatrix * gl_Normal);
	
	lightPos = vec3(gl_ModelViewMatrix * gl_LightSource[0].position);
	
	color = gl_Color;
	
	gl_FrontColor = gl_Color;
	
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}	