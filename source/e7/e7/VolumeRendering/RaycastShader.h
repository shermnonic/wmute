// Max Hermann, March 22, 2010
#ifndef RAYCASTSHADER_H
#define RAYCASTSHADER_H

#include <GL/GLSLProgram.h>
#include <GL/GLError.h>
#include <GL/GLTexture.h>
#include <cassert>

/// One pass raycasting shader supporting displacement vectorfields 
/// Main purpose of this class is to bundle GLSL program and uniform parameters.
class RaycastShader
{
public:
	enum RenderMode {
		RenderDirect    =0, ///< Direct Volume Rendering (accumulate intensity along ray)
		RenderIsosurface=1, ///< Non-polygonal Isosurface (stop ray at specific intensity)
		RenderMIP       =2, ///< Maximum-intensity-projection
		RenderSilhouette=3, ///< Render simple silhouette
		NumRenderModes
	};

	enum Constants {
		MaxWarpModes = 5
	};

	RaycastShader()
		: m_shader      (NULL)
		 ,m_rendermode  (RenderDirect)
		 ,m_num_channels(1)
		 ,m_warp        (false)
		 ,m_multiwarp   (false)
		 ,m_meanwarp    (false)
		 ,m_stepsize    (0.005f) // good default for my application
		 ,m_isovalue    (0.042f) // dito
		 ,m_alpha_scale (0.010f) // dito
		 ,m_fs_path("shader/raycast.fs.glsl")
		 ,m_vs_path("shader/raycast.vs.glsl")
		{
			m_size[0] = m_size[1] = m_size[2] = 128;
			m_warp_ofs[0] = m_warp_ofs[1] = m_warp_ofs[2] = 0;
			for( int i=0; i < MaxWarpModes; ++i )
				m_lambda[i] = 0.f;
		}
	~RaycastShader() { if(m_shader) delete m_shader; }

	/// configure shader before calling init() with configuration setters!
	bool init( std::string fs_path = "", std::string vs_path = "" );
	void deinit(); // destruction of shader only in GL context possible

	//void set_frontbacktex( GLTexture* front, GLTexture* back );
	//void set_voltex( GLTexture* 

	/// bind shader and set used texture units
	/// \param firsttexunit first texture unit
	/// Texture unit layout starting from firsttexunit:
	///  - +0 : front texture
	///  - +1 : back texture
	///  - +2 : volume texture
	///  - warp textures follow after volume texture, then meanwarp.
	///  - last texture is the depth texture.
	void bind( GLuint firsttexunit=0 );
	void release();

	///@{ Configuration (will be applied on call to \a init())
	void  set_volume_size( int width, int height, int depth )
		{
			m_size[0] = width;
			m_size[1] = height;
			m_size[2] = depth;
		}
	void  set_num_channels( int nc ) { m_num_channels = nc; }
	int   get_num_channels() const { return m_num_channels; }
	void  set_rendermode( int/*RenderMode*/ mode )
		{
			m_rendermode = mode;
		}
	/*RenderMode*/int get_rendermode() const
		{
			return m_rendermode;
		}
	void  cycle_rendermode()
		{
			m_rendermode = static_cast<RenderMode>( ((int)m_rendermode+1) % NumRenderModes );
		}
	void  set_warp_enabled( bool b ) { m_warp = b; }
	bool  get_warp_enabled() const { return m_warp; }
	void  set_multiwarp_enabled( bool b ) { m_multiwarp = b; }
	bool  get_multiwarp_enabled() const { return m_multiwarp; }
	void  set_depth_enabled( bool b ) { m_depth = b; }
	bool  get_depth_enabled( bool b ) const { return m_depth; }
	void  set_meanwarp_enabled( bool b ) { m_meanwarp = b; }
	bool  get_meanwarp_enabled() const { return m_meanwarp; }
	///@}

	///@{ Uniforms
	void  set_isovalue( float iso ) { m_isovalue = iso; }
	float get_isovalue() const { return m_isovalue; }
	void  set_alpha_scale( float sc ) { m_alpha_scale = sc; }
	float get_alpha_scale() const { return m_alpha_scale; }

	void  set_lambda( int i, float s ) { assert(i>=0&&i<MaxWarpModes); m_lambda[i]=s; }
	float get_lambda( int i ) const { assert(i>=0&&i<MaxWarpModes); return m_lambda[i]; }

	void  set_stepsize( float s ) { m_stepsize=s; }
	float get_stepsize() const { return m_stepsize; }

	// for backwards compatibility
	void  set_warp_strength( float s ) { set_lambda(0,s); }
	float get_warp_strength() const { return get_lambda(0); }
	void  set_warp_ofs( float dx, float dy, float dz )
	{
		m_warp_ofs[0] = dx;
		m_warp_ofs[1] = dy;
		m_warp_ofs[2] = dz;
	}
	void  set_warp_ofs( float* ofs )
	{
		m_warp_ofs[0] = ofs[0];
		m_warp_ofs[1] = ofs[1];
		m_warp_ofs[2] = ofs[2];
	}
	void get_warp_ofs( float* ofs ) const
	{
		ofs[0] = m_warp_ofs[0];
		ofs[1] = m_warp_ofs[1];
		ofs[2] = m_warp_ofs[2];
	}
	void get_warp_ofs( float& dx, float& dy, float& dz )
	{
		dx = m_warp_ofs[0];
		dy = m_warp_ofs[1];
		dz = m_warp_ofs[2];
	}
	///@}

private:
	GL::GLSLProgram*  m_shader;
	GLint             m_loc_warpmode[MaxWarpModes],
		              m_loc_lambda  [MaxWarpModes],
		              m_loc_voltex,
					  m_loc_meanwarptex,
	                  m_loc_fronttex,
	                  m_loc_backtex,
					  m_loc_isovalue,
					  m_loc_alpha_scale,
	                  m_loc_warp_ofs,
					  m_loc_depthtex,
					  m_loc_stepsize;
	// configuration
	int  m_size[3];
	int  m_rendermode;
	int  m_num_channels;
	bool m_warp;
	bool m_multiwarp;
	bool m_meanwarp;
	bool m_depth;

	// variables
	float  m_stepsize;
	float  m_isovalue;
	float  m_alpha_scale;
	float  m_lambda[MaxWarpModes];
	float  m_warp_ofs[3];

	std::string m_fs_path, 
		        m_vs_path;
};

#endif
