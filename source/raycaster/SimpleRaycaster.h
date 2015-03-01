#ifndef SIMPLERAYCASTER_H
#define SIMPLERAYCASTER_H

#include "GLConfig.h"
#include "GLTexture.h"
#include "RaycastShader.h"
#include "RenderToTexture.h"

class LookupTable;

class SimpleRaycaster
{
public:
	SimpleRaycaster()
		: m_lut(NULL), m_lut_size(256)
		{}

	bool init( int width, int height );
	void destroy();
	
	bool downloadVolume( int resX, int resY, int resZ, GLenum elementType,
	                     void* data );

	void setSpacing( float spacingX, float spacingY, float spacingZ );

	void setLookupTable( LookupTable* table );

	void render();

	RaycastShader& getRaycastShader() { return m_raycast_shader; }

protected:
	void generateStartEndPositions();
	void traverseRays();

	void drawRGBCube();
	void drawTexQuad( int w, int h, GLenum unit=GL_TEXTURE0 );

private:
	GL::GLTexture   m_volume_tex,      ///< 3D image data
	                m_back_tex,        ///< Ray end positions
			        m_front_tex,       ///< Ray start positions
	                m_screen_tex,      ///< Raycasted result image
					m_lut_tex;         ///< RGBA 1D Color lookup table
	RaycastShader   m_raycast_shader;
	RenderToTexture m_r2t;
	int             m_width, m_height; ///< Internal texture size
	int             m_res[3];          ///< Image volume resolution
	float           m_spacing[3];      ///< Image volume spacing
	LookupTable*    m_lut;
	int             m_lut_size;

};

#endif // SIMPLERAYCASTER_H
