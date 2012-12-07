#ifndef SCREENSHOT2_H
#define SCREENSHOT2_H

#include <string>

void grabTGA( std::string filename, bool fromActiveTexture=false,
			  int width=-1, int height=-1 );
std::string autoName( std::string prefix, std::string postfix );

class Screenshot2
{
public:
	Screenshot2();

	void setup( int width, int height, std::string prefix="Screenshot-" );
	void destroy();

	void render( void (*renderFunc)(), void (*reshapeFunc)(int,int)=NULL );

	void begin();
	void end();

	int width() const { return m_width; }
	int height() const { return m_height; }

	std::string getLastFilename() const { return m_lastFilename; };

	void setPrefix( std::string prefix ) { m_prefix = prefix; }

	bool isInitialized() const { return m_initialized; }
	std::string prefix() const { return m_prefix; }

private:
	bool m_initialized;
	int m_width, m_height;
	std::string m_prefix;
	std::string m_lastFilename;
	unsigned int m_texid, m_texunit, m_fbo;	
};

#endif // SCREENSHOT2_H
