// Platform independent way to include basic OpenGL headers.
// See also:
// http://stackoverflow.com/questions/3907818/opengl-headers-for-os-x-linux
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#ifdef _WIN32
  #include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif
