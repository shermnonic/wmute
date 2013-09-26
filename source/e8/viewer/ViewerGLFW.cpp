#include "ViewerGLFW.h"

//--- ViewerWindowMap helper class ---

void ViewerGLFW::ViewerWindowMap::
  addViewer( ViewerGLFW* viewer, GLFWwindow* window )
{
	m_viewer2window[viewer] = window;
	m_window2viewer[window] = viewer;
}
		
void ViewerGLFW::ViewerWindowMap::
  removeViewer( ViewerGLFW* viewer )
{			
	ViewerToWindowMap::iterator it;
	it = m_viewer2window.find( viewer );
	if( it != m_viewer2window.end() )
	{
		GLFWwindow* window = it->second;
		
		WindowToViewerMap::iterator it2;
		it2 = m_window2viewer.find( window );
		if( it2 != m_window2viewer.end() )
			m_window2viewer.erase( window );
		
		m_viewer2window.erase( viewer );
	}	
}
		
ViewerGLFW* ViewerGLFW::ViewerWindowMap::
  getViewer( GLFWwindow* window )
{
	WindowToViewerMap::iterator it2;
	it2 = m_window2viewer.find( window );
	if( it2 != m_window2viewer.end() )
		return it2->second; // ViewerGLFW*
	
	// Not found
	return NULL;
}


//--- Static members and GLFW callbacks ---
		
ViewerGLFW::ViewerWindowMap ViewerGLFW::s_viewerWindowMap;

void ViewerGLFW::
  s_cursorposfun( GLFWwindow* w, double xpos, double ypos )
{
	ViewerGLFW* viewer = getViewer( w );
	if( viewer )
		viewer->cursorposfun( w, xpos, ypos );
}

void ViewerGLFW::
  s_keyfun( GLFWwindow* w, int key, int scancode, int action, int mods )
{
	ViewerGLFW* viewer = getViewer( w );
	if( viewer )
		viewer->keyfun( w, key, scancode, action, mods );
}

void ViewerGLFW::
  s_mousebuttonfun( GLFWwindow* w, int button, int action, int mods )
{
	ViewerGLFW* viewer = getViewer( w );
	if( viewer )
		viewer->mousebuttonfun( w, button, action, mods );
}

void ViewerGLFW::
  s_scrollfun( GLFWwindow* w, double xoffset, double yoffset )
{
	ViewerGLFW* viewer = getViewer( w );
	if( viewer )
		viewer->scrollfun( w, xoffset, yoffset );
}


//--- ViewerGLFW ---

ViewerGLFW::ViewerGLFW( GLFWwindow* window )
  : m_window(window),
    m_renderer(NULL),
	m_interactor(NULL)
{
	// Add global pointer to map
	s_viewerWindowMap.addViewer( this, window );

	// Set GLFW callbacks
	setupGLFWCallbacks();
}

ViewerGLFW::~ViewerGLFW()
{
	// Remove global pointer from map
	s_viewerWindowMap.removeViewer( this );
}


void ViewerGLFW::
  setupGLFWCallbacks()
{
	glfwSetKeyCallback        ( m_window, &ViewerGLFW::s_keyfun );
	glfwSetMouseButtonCallback( m_window, &ViewerGLFW::s_mousebuttonfun );
	glfwSetScrollCallback     ( m_window, &ViewerGLFW::s_scrollfun );
	glfwSetCursorPosCallback  ( m_window, &ViewerGLFW::s_cursorposfun );
}

void ViewerGLFW::
  cursorposfun( GLFWwindow* w, double xpos, double ypos )
{
}

void ViewerGLFW::
  keyfun( GLFWwindow* w, int key, int scancode, int action, int mods )
{
}

void ViewerGLFW::
  mousebuttonfun( GLFWwindow* w, int button, int action, int mods )
{
}

void ViewerGLFW::
  scrollfun( GLFWwindow* w, double xoffset, double yoffset )
{
}
