#ifndef TRACKBALL_H
#define TRACKBALL_H
#include <GL/glut.h>


typedef struct 
{
	int x;
	int y;
}	Point;



class Trackball
{
public:
	void start(Point point);
	void rollTo(Point point, GLfloat *r);
	void addToRotation(GLfloat *da, GLfloat *A);



protected:
	GLfloat _radius, _startPt[3], _endPt[3];
	Point _ctr;

	void rotation2Quat(GLfloat *A, GLfloat *q);
};



class SimpleTrackball : public Trackball {

 public:

  SimpleTrackball(){
  	_rotation[0] = _tbRot[0] = 0.0;
	_rotation[1] = _tbRot[1] = 1.0;
	_rotation[2] = _tbRot[2] = 0.0;
	_rotation[3] = _tbRot[3] = 0.0;
	_drag = false;
	_zoom = false;
	_zoomFac = 1;
	_zoomTemp=0.f;
  }

 
  // in der glutMouseFunc aufrufen, wenn state == GLUT_DOWN
  void startBall(int x, int y);  

  // in der glutMouseFunc aufrufen, wenn state == GLUT_UP
  void stopBall(int x, int y);

  void startZoom(int x, int y);
  void stopZoom(int x, int y);  
  
  // in der glutMotionFunc aufrufen
  void rollBall(int x, int y);

  // multipiziert die Trackballrotation auf die aktive Matrix
  // (z.B. in der display Funktion aufrufen)
  void multModelMatrix();


 private:
  void rotateBy(GLfloat *r);
  GLfloat _rotation[4], _tbRot[4];
  GLfloat _zoomFac, _zoomTemp;
  bool _drag;
  bool _zoom;
};

#endif
