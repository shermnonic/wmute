#ifndef PARTICLEMODULE_H
#define PARTICLEMODULE_H

#include "ModuleRenderer.h"
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
	void applyOptions() { /* Call init again to change texture size */ init(); }
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
	// Some initialization has only to be done once
	bool            m_target_initialized;
	bool            m_r2t_initialized;
	bool            m_ps_initialized;

	bool            m_update;
	GLTexture       m_target;
	RenderToTexture m_r2t;
	ParticleSystem  m_ps;

	/// Live parameters
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

	/// Setup options
	struct Opts
	{
		IntParameter width, height;
		Opts() 
		: width("targetWidth"),
		  height("targetHeight")
		{
			width.setValueAndDefault( 1024 );
			width.setLimits( 1, 2048 );
			height.setValueAndDefault( 1024 );
			height.setLimits( 1, 2048 );
		}
	};
	Opts m_opts;
};


#endif // PARTICLEMODULE_H
