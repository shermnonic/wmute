#ifndef PARTICLEMODULE_H
#define PARTICLEMODULE_H

#include "RenderSet.h" // for ModuleRenderer
#include "ParticleSystem.h"

#include <glutils/GLTexture.h>
#include <glutils/RenderToTexture.h>
#ifdef GL_NAMESPACE
using GL::GLTexture;
using GL::GLSLProgram;
#endif

/**
	\class ParticleModule

	Simple GPU based state-preserving particle system.
*/
class ParticleModule : public ModuleRenderer
{
public:
	ParticleModule();

	bool init();

	///@name ModuleRenderer implementation
	///@{
	void render();
	int  target() const { return m_target.GetID(); }
	void destroy() { m_ps.destroy(); }
	void touch() { m_ps.touch(); m_update=true; }
	///@}
	
	/// @name Serialization
	///@{
	PropertyTree& serialize() const;
	void deserialize( Serializable::PropertyTree& pt );
	///@}	
	
private:
	bool            m_initialized;
	bool            m_update;
	int             m_width, m_height;
	GLTexture       m_target;
	RenderToTexture m_r2t;
	ParticleSystem  m_ps;
};


#endif // PARTICLEMODULE_H
