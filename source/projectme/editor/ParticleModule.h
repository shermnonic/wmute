#ifndef PARTICLEMODULE_H
#define PARTICLEMODULE_H

#include "RenderSet.h" // for ModuleRenderer
#include "Parameter.h"
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
	typedef ModuleRenderer Super;

	ParticleModule();

	bool init();

	void setForceTexture( int texid ) { m_ps.setForceTexture(texid); }

	///@name ModuleRenderer implementation
	///@{
	void render();
	int  target() const { return m_target.GetID(); }
	void destroy() { m_ps.destroy(); }
	void touch();
	///@}

	///@name ModuleRenderer channels implementation
	///@{
	void setChannel( int idx, int texId );
	int  channel( int idx ) const;
	int  numChannels() const { return 2; }
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

	// Live parameters
	struct Params {
		DoubleParameter pointSize;
		Params()
		: pointSize("PointSize")
		{
			pointSize.setValueAndDefault( 1.5 );
			pointSize.setLimits( 0.1, 50.0 );
		}
	};
	Params m_params;
};


#endif // PARTICLEMODULE_H
