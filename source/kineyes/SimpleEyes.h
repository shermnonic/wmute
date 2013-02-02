#ifndef SIMPLEEYES_H
#define SIMPLEEYES_H

class SimpleEye
{
public:
	SimpleEye()
	: m_initialized(false),
	  m_eyeRadius(1.)
	{
		setPosition(0,0,0);
		setPOI(0,0,1);
	}
	
	void init();
	void destroy();

	void draw();
	
	void setPosition( float x, float y, float z )
		{ m_posx=x; m_posy=y; m_posz=z; }
	void setPOI( float x, float y, float z )
		{ m_poix=x; m_poiy=y; m_poiz=z; }
	void setRadius( float r )
		{ m_eyeRadius = r; }

private:	
	bool  m_initialized;
	int   m_eyeBall;   // display list for eyeball
	int   m_eyePupille;
	float m_eyeRadius; // radius of eyeball
	float m_posx, m_posy, m_posz;
	float m_poix, m_poiy, m_poiz;
};

/// Two instances of \a SimpleEye aligned along x
class SimpleEyes
{
public:
	SimpleEyes()
		: m_eyeSep(1.5)
	{}

	void init()
	{
		m_leftEye .init();
		m_rightEye.init();
	}

	void destroy()
	{
		m_leftEye .destroy();
		m_rightEye.destroy();
	}

	void draw()
	{
		m_leftEye .draw();
		m_rightEye.draw();
	}

	void setPosition( float x, float y, float z )
	{ 
		m_leftEye .setPosition( x-m_eyeSep, y, z );
		m_rightEye.setPosition( x+m_eyeSep, y, z );
		m_posx=x; m_posy=y; m_posz=z;
	}

	void setPOI( float x, float y, float z )
	{ 
		m_leftEye.setPOI(x,y,z); 
		m_rightEye.setPOI(x,y,z);
	}

	void setRadius( float r )
	{
		m_leftEye.setRadius( r );
		m_rightEye.setRadius( r );
	}

	void setSeparation( float sep )
	{
		m_eyeSep = sep;
		setPosition( m_posx, m_posy, m_posz );
	}

private:
	SimpleEye m_leftEye, m_rightEye;
	float m_eyeSep; // eye separation, i.e. distance between eyeballs
	float m_posx, m_posy, m_posz;
};

#endif // SIMPLEEYES_H
