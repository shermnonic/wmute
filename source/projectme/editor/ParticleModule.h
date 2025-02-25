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
	int  target() const { return m_target.name(); }
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
        EnumParameter blendMode;
        DoubleParameter fraction;
        EnumParameter animation;
        DoubleParameter animSpeed;
        DoubleParameter timestep;
		Params()
        : pointSize("PointSize"),
          blendMode("BlendMode","None","Alpha","Over"),
          fraction("Fraction"),
          animation("Animation","Static","In","Out"),
          animSpeed("AnimSpeed"),
          timestep("Timestep")
		{
			pointSize.setValueAndDefault( 1.5 );
			pointSize.setLimits( 0.1, 50.0 );
            blendMode.setValue( 1 );
            fraction.setValueAndDefault( 1.0 );
            fraction.setLimits( 0.0, 1.0 );
            animation.setValue( 0 );
            animSpeed.setValueAndDefault( 0.1 );
            animSpeed.setLimits( 0.0, 100.0 );
            timestep.setValueAndDefault( 0.15 ); // 100* original dt
            timestep.setLimits( 0.001, 10.0 );
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
