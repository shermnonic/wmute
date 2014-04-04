// scene - minimalistic scene graph library
// Max Hermann, Jan 2014
#include "Scene.h"
#include <glutils/GLConfig.h> // replacement for include<GL/GL.h>
#include <glutils/GLError.h>
#include <sstream>

namespace scene
{	
//-----------------------------------------------------------------------------
void Scene::render( int flags )
{
	ObjectPtrArray::iterator it = m_objects.begin();
	for( unsigned i=0; it != m_objects.end(); ++it, i++ )
	{
		GL::CheckGLError("Scene::render() - start single object");

		if( flags & Object::RenderNames )
		{
			glPushName( i );  
			GL::CheckGLError("Scene::render() - single object glPushName()");
		}

		if( (*it)->isVisible() )
			(*it)->render( flags );
		
		GL::CheckGLError("Scene::render() - single object render()");

		if( flags & Object::RenderNames )
		{
			glPopName();      
			GL::CheckGLError("Scene::render() - single object  glPopName()");
		}
	}
}

//-----------------------------------------------------------------------------
void Scene::addSceneObject( ObjectPtr obj )
{
	m_objects.push_back( obj );
}

//-----------------------------------------------------------------------------
void Scene::removeSceneObject( int i )
{
	m_objects.erase( m_objects.begin() + i );
}

//-----------------------------------------------------------------------------
BoundingBox Scene::getBoundingBox() const
{
	BoundingBox bbox;
	ObjectPtrArray::const_iterator it = m_objects.begin();
	for( ; it != m_objects.end(); ++it )
	{
		bbox.include( (*it)->getBoundingBox() );
	}
	return bbox;
}

} // namespace scene
