#ifndef ILLUMINATEDLINESRENDERER_H
#define ILLUMINATEDLINESRENDERER_H

#include <GL/glew.h>
#include <glutils/GLSLProgram.h>
#include <glutils/GLError.h>
#include <glutils/GLTexture.h>

/// Simple tangent color shader
class TangentColorShader
{
	static std::string s_vshader, s_fshader;
	
public:
	TangentColorShader(): m_shader(NULL) {}
	~TangentColorShader() { deinit(); }

	bool init();
	void deinit();

	void bind( GLuint shadingTex=0 );
	void release();

private:
	GL::GLSLProgram *m_shader;
};

/// Helper class for \a IlluminatedLinesRenderer
class IlluminatedLinesShader
{
	static std::string s_vshader, s_fshader;
	
public:
	IlluminatedLinesShader();
	~IlluminatedLinesShader();

	bool init();
	void deinit();

	void bind( GLuint shadingTex=0 );
	void release();

private:
	GL::GLSLProgram *m_shader;
	GL::GLTexture    m_shadtex; // shading texture
	GLint m_loc_shadtex;
};

/// Render linestrips phong lighted approximately like an infinitesimal cylinder
class IlluminatedLinesRenderer
{
public:
	bool init();
	void destroy();
	void render();

private:
	IlluminatedLinesShader m_shader;
};

#endif // ILLUMINATEDLINESRENDERER_H
