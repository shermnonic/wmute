// This is the general OpenGL include file which should be used instead of GL.h.
#ifndef GL_GLCONFIG_H
#define GL_GLCONFIG_H

// We use glew for OpenGL extension handling which includes itself GL.h
#include <GL/glew.h>

// Put our own OpenGL functions in the namespace GL:: 
#define GL_NAMESPACE

#endif
