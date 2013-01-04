// Max Hermann, June 6, 2010
#ifndef VOLUMERENDERERRAYCAST_H
#define VOLUMERENDERERRAYCAST_H

#include <GL/GLTexture.h>
#include <GL/RenderToTexture.h>
#include "ClipCube.h"
#include "RaycastShader.h"

class VolumeRendererRaycast
{
public:
	VolumeRendererRaycast();
	virtual ~VolumeRendererRaycast();

	/// Initialize raycast shader (should only be called once, see also \a reinit_shader())
	/// \param textWidth 	size of render texture
	/// \param textHeight 	size of render texture
	bool init( int texWidth=512, int texHeight=512 );
	void destroy();

	void render();

	/// Reload shader
	// Q: Really needed to be public? Currently yes, because the setters don't
	//    reload the shader and init() should only be called once!
	bool reinit() { return reinit_shader(); }

	/// Set volume texture
	void setVolume( GL::GLTexture* vtex );

	/// Set single warpfield and enable corresponding visualization mode
	void setWarpfield( GL::GLTexture* warp );

	/// Set multiple warpfields and enable corresponding visualization mode
	void setWarpfields( GL::GLTexture* mode0, GL::GLTexture* mode1, GL::GLTexture* mode2,
		                GL::GLTexture* mode3, GL::GLTexture* mode4 );

	/// Set mean warpfield, which will be added to any reconstructions
	void setMeanwarp( GL::GLTexture* meanwarp );

	void setAspect( float ax, float ay, float az );
	void getAspect( float& ax, float& ay, float &az ) const;

	///@{ Convenience setters/getters forwarded to/from \a RaycastShader
	void setRenderMode( RaycastShader::RenderMode mode );
	void setIsovalue  ( float iso );
	void setZNear     ( float znear );
	float getIsovalue  () const;
	int   getRenderMode() const;
	///@}

	RaycastShader& getRaycastShader() { return m_raycast_shader; }
	const RaycastShader& getRaycastShader() const { return m_raycast_shader; }

	void changeTextureSize( int width, int height );
	int  getTextureWidth () const { return m_front.GetWidth(); }
	int  getTextureHeight() const { return m_front.GetHeight(); }
protected:
	bool createTextures( int texWidth, int texHeight );
	void destroyTextures();
	bool createRenderToTexture( int texWidth, int texHeight );
	void destroyRenderToTexture();

public:
	void setOffscreenTextureSize( int width, int height );

	void setOffscreen( bool b ) { m_offscreen = b; }
	void setDebug( bool b ) { m_debug = b; }
	bool getOffscreen() const { return m_offscreen; }
	bool getDebug() const { return m_debug; }
	int  getOffscreenWidth() const { return m_vren.GetWidth(); }
	int  getOffscreenHeight() const { return m_vren.GetHeight(); }

	int getOffscreenTexture() { return m_vren.GetID(); }
	RenderToTexture* getR2T() { return &m_r2t; }

protected:
	void reshape_ortho( int w, int h );
	bool reinit_shader( std::string fs_path="", std::string vs_path="" );

private:
	int m_verbosity;

	GL::GLTexture   m_front,  // front cube(s) (ray start positions)
	                m_back,   // back cube(s) (ray end positions)
					m_vren;   // raycasted volume (if not directly rendered)
	ClipCube        m_cube;
	RenderToTexture m_r2t;
	RaycastShader   m_raycast_shader;

	// 3D textures
	GL::GLTexture *m_vtex,     // scalar field (required)
	              *m_warps[5], // displacement field(s) (optional)
	              *m_meanwarp; // mean displacement field (optional with m_warp)	
	// Note:
	// Warp stuff still experimental! 
	// Should probably be put into own class/shader.

	float m_aspect[3];     // volume aspect ratio
	float m_alpha;         // alpha scaling factor for direct volume rendering
	                       // TODO: Inconsistent to store alpha value redundantly
	                       //       here and in RaycastShader.
	float m_znear;         // znear is needed for drawing near plane

	bool m_offscreen;
	bool m_debug;
};

#endif // VOLUMERENDERERRAYCAST_H
