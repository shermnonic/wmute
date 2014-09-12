#include "Trackball.h"

#include<cmath>

using std::sqrt;

static const float kTOL = 0.001;
static const float kRAD2DEG = 180. / 3.1415927;
static const float kDEG2RAD = 3.1415927 / 180.;

void Trackball::start(Point point)
{
/*
	Start the trackball.  The trackball pretends that a ball encloses the 3d view.  
	You roll this ball with the mouse.  For example, if you click on the center
	of the ball and move the ball directly right, you rotate around y.  Click on
	edge of ball and roll to get a z rotation.

	The idea isn't too hard.  Start with a vector from the first mouse click to the
	center of the 3d view.  Set the radius of the ball to the smaller dimension of the
	3d view.  As you drag around, a second vector is determined from the surface to center
	of the ball.  Axis of rotation is cross-product of those two vectors, and the angle
	is the angle between the vectors.
*/

	GLfloat xxyy, nx, ny;
	nx = glutGet(GLUT_WINDOW_WIDTH);
	ny = glutGet(GLUT_WINDOW_HEIGHT);
	if (nx > ny)
		_radius = ny * 0.5;
	else
		_radius = nx * 0.5;

	//center of view
	_ctr.x = int(double(glutGet(GLUT_WINDOW_WIDTH)) * 0.5);
	_ctr.y = int(double(glutGet(GLUT_WINDOW_HEIGHT)) * 0.5);

	//starting vector from surface of ball to center
	_startPt[0] = point.x - _ctr.x;
	_startPt[1] = _ctr.y - point.y;
	xxyy = _startPt[0] * _startPt[0] + _startPt[1] * _startPt[1];
	if (xxyy > _radius * _radius)
		//outside sphere
		_startPt[2] = 0;
	else
		_startPt[2] = sqrt(_radius * _radius - xxyy);
}

void Trackball::rollTo(Point point, GLfloat *r)
{
	float xxyy, rot[4], cosAng, sinAng, ls, le, lr;

	_endPt[0] = point.x - _ctr.x;
	_endPt[1] = _ctr.y - point.y;
	//below tolerance
	if (fabs(_endPt[0] - _startPt[0]) < kTOL && fabs(_endPt[1] - _startPt[1]) < kTOL)
		return;		
	
	//ending vector from surface of ball to center
	xxyy = _endPt[0] * _endPt[0] + _endPt[1] * _endPt[1];
	if (xxyy > _radius * _radius)
		_endPt[2] = 0; //outside sphere
	else
		_endPt[2] = sqrt(_radius * _radius - xxyy);

	//cross vectors rot = st x end
	rot[1] = _startPt[1] * _endPt[2] - _startPt[2] * _endPt[1];
	rot[2] = -1 * _startPt[0] * _endPt[2] + _startPt[2] * _endPt[0];
	rot[3] = _startPt[0] * _endPt[1] - _startPt[1] * _endPt[0];

	//using atan since sin and cos gives rotations that can flip near poles
	//cos(a) = (s . e)/(||s|| ||e||)
	//s . e
	cosAng = _startPt[0] * _endPt[0] + _startPt[1] * _endPt[1] + _startPt[2] * _endPt[2];
	ls = sqrt(_startPt[0] * _startPt[0] + _startPt[1] * _startPt[1] + _startPt[2] * _startPt[2]);
	ls = 1./ls; //1 / ||s||
	le = sqrt(_endPt[0] * _endPt[0] + _endPt[1] * _endPt[1] + _endPt[2] * _endPt[2]);
	le = 1./le; //1 / ||e||
	cosAng = cosAng * ls * le;

	//sin = ||(s x e)||/(||s|| ||e||)
	sinAng = lr = sqrt(rot[1] * rot[1] + rot[2] * rot[2] + rot[3] * rot[3]); //||(s x e)||
	sinAng = sinAng * ls * le;
	rot[0] = static_cast<GLfloat>(atan2(sinAng, cosAng) * kRAD2DEG);
	
	//Normalize rot axis
	lr = 1/lr;
	int i;
	for (i = 1; i < 4; i++)
		rot[i] *= lr;

	for (i = 0; i < 4; i++)
		r[i] = rot[i];
}

void Trackball::rotation2Quat(GLfloat *A, GLfloat *q)
{
	GLfloat ang2, sinAng2; //half angles

	//convert GL rotation to a quaternion.  GL looks like: {ang, x, y, z}
	//and quat looks like: {{v}, cos(angle/2)} where {v} is (x,y,z)/sin(angle/2)

	ang2 = A[0] * kDEG2RAD * 0.5;
	sinAng2 = sin(ang2);
	q[0] = A[1] * sinAng2;
	q[1] = A[2] * sinAng2;
	q[2] = A[3] * sinAng2;
	q[3] = cos(ang2);
}

void Trackball::addToRotation(GLfloat *da, GLfloat *A)
{
	GLfloat q0[4], q1[4], q2[4], theta2, sinTheta2;
	//A' = A . da
	//for quaternions: let q0 <- A, and q1 <- dA.
	//Figure out: q2 = q1 + q0 (note order)
	
	//A' <- q3
	rotation2Quat(A, q0);
	rotation2Quat(da, q1);

	//q2 = q1 + q0
	q2[0] = q1[1]*q0[2] - q1[2]*q0[1] + q1[3]*q0[0] + q1[0]*q0[3];
	q2[1] = q1[2]*q0[0] - q1[0]*q0[2] + q1[3]*q0[1] + q1[1]*q0[3];
	q2[2] = q1[0]*q0[1] - q1[1]*q0[0] + q1[3]*q0[2] + q1[2]*q0[3];
	q2[3] = q1[3]*q0[3] - q1[0]*q0[0] - q1[1]*q0[1] - q1[2]*q0[2];

	//Identity rotation is rot by 0 about an axis. "angle" in quaternion is
	//actually cos of 1/2 angle.  So, if cos 1/2 angle's 1 (or within tolerance)
	//then you have an identity rotation
	if (fabs(fabs(q2[3] - 1.)) < 1.0e-7)
	{
		//id rotation
		A[0] = 0;
		A[1] = 1;
		A[2] = A[3] = 0.;
		return;
	}

	//here is non-identity rotation.  cos of half-angle is non-zero, so sine is non-zero
	//Therefore, we can divide by sin(theta2) w/o fear
	
	//turn quat. back to angle/axis rotation
	theta2 = acos(q2[3]);
	sinTheta2 = 1./sin(theta2);
	A[0] = theta2 * 2. * kRAD2DEG;
	A[1] = q2[0] * sinTheta2;
	A[2] = q2[1] * sinTheta2;
	A[3] = q2[2] * sinTheta2;
}


void SimpleTrackball::startBall(int x, int y)
{
	Point temp;
	temp.x = x;
	temp.y = y;
	start(temp);
	_drag = true;

}

void SimpleTrackball::stopBall(int x, int y)
{
	addToRotation(_tbRot, _rotation);
	_tbRot[0] = 0.0;
	_tbRot[1] = 1.0;
	_tbRot[2] = 0.0;
	_tbRot[3] = 0.0;
	_drag = false;
}

void SimpleTrackball::startZoom(int x, int y)
{
  _startPt[1] = y;
  _zoom = true;
}

void SimpleTrackball::stopZoom(int x, int y)
{
  _zoomFac += _zoomTemp;
  _zoomTemp=0.f;
  _zoom = false;
}



void SimpleTrackball::rollBall(int x, int y)
{
    
  if(_drag){
    GLfloat rot[4];
    Point temp;
    temp.x = x;
    temp.y = y;
    rollTo(temp, rot);
    rotateBy(rot);
  }
  else if(_zoom){
    _zoomTemp = (y-_startPt[1]) / (float)glutGet(GLUT_WINDOW_HEIGHT);
  }
}

void SimpleTrackball::rotateBy(GLfloat *r)
{
	for (int i = 0; i < 4; i++)
		_tbRot[i] = r[i];
}



void SimpleTrackball::multModelMatrix(){
	glRotatef(_tbRot[0], _tbRot[1], _tbRot[2], _tbRot[3]);
	glRotatef(_rotation[0], _rotation[1], _rotation[2], _rotation[3]);
	glScalef(_zoomFac+_zoomTemp,_zoomFac+_zoomTemp,_zoomFac+_zoomTemp); 
}
