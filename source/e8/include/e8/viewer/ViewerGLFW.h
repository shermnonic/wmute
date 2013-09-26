#ifndef VIEWERGLFW_H
#define VIEWERGLFW_H

#include <e8/base/ViewerInterface.h>
#include <GLFW/glfw3.h>
#include <map>

/// GLFW based e8 viewer
class ViewerGLFW : public ViewerInterface
{
	/// Helper class to store viewer / window pairs
	/// Instead of the global ViewerGLFW instance pointer we could also use
	/// glfwGetWindowUserPointer() to retrieve a per-window pointer.
	class ViewerWindowMap
	{
		typedef std::map< ViewerGLFW*, GLFWwindow* >  ViewerToWindowMap;
		typedef std::map< GLFWwindow*, ViewerGLFW* >  WindowToViewerMap;
	private:
		ViewerToWindowMap m_viewer2window;
		WindowToViewerMap m_window2viewer;	
	public:
		void addViewer( ViewerGLFW* viewer, GLFWwindow* window );		
		void removeViewer( ViewerGLFW* viewer );		
		ViewerGLFW* getViewer( GLFWwindow* window );
	};
	
	/// Store viewer / window pairs
	static ViewerWindowMap s_viewerWindowMap;
	///@{ Convenience forwards from ViewerWindowMap static instance
	static ViewerGLFW* getViewer( GLFWwindow* w ) 
		{ return s_viewerWindowMap.getViewer(w); }
	///@}

public:
	ViewerGLFW( GLFWwindow* window );	
	~ViewerGLFW();
	
public:
	///@{ ViewerInterface implementation
	void setRenderer( AbstractRenderer* renderer ) { m_renderer = renderer; }
	void setInteractor( AbstractInteractor* interactor ) { m_interactor = interactor; }
	///@}
	
protected:
	void setupGLFWCallbacks();

	///@{ GLFW callbacks (called from static callbacks)
	void cursorposfun( GLFWwindow* w, double xpos, double ypos );
	void keyfun( GLFWwindow* w, int key, int scancode, int action, int mods );
	void mousebuttonfun( GLFWwindow* w, int button, int action, int mods );
	void scrollfun( GLFWwindow* w, double xoffset, double yoffset );	
	///@}

	///@{ GLFW callbacks (static functions)
	static void s_cursorposfun( GLFWwindow* w, double xpos, double ypos );
	static void s_keyfun( GLFWwindow* w, int key, int scancode, int action, int mods );
	static void s_mousebuttonfun( GLFWwindow* w, int button, int action, int mods );
	static void s_scrollfun( GLFWwindow* w, double xoffset, double yoffset );
	///@}

private:
	GLFWwindow* m_window;	
	AbstractRenderer*   m_renderer;
	AbstractInteractor* m_interactor;
};

#endif // VIEWERGLFW_H
