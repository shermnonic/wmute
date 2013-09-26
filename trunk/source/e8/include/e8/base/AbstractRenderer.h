#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

class AbstractRenderer // : public Module
{
public:
	virtual void initialize()=0;
	virtual void update( float t )=0;
	virtual void render()=0;
	virtual void resize( int w, int h )=0;
};

#endif // ABSTRACTRENDERER_H
