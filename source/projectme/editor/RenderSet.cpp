#include "RenderSet.h"
#include "glbase.h"

//=============================================================================
//  RenderArea
//=============================================================================

//-----------------------------------------------------------------------------
void RenderArea::drawAreaOutline() const
{
	// Draw line
	glBegin( GL_LINE_LOOP );
	for( int i=0; i < m_poly.nverts(); i++ )
		glVertex2fv( m_poly.vert(i) );
	glEnd();
	
	// Draw handles
	glBegin( GL_POINTS );
	for( int i=0; i < m_poly.nverts(); i++ )
		glVertex2fv( m_poly.vert(i) );
	glEnd();
}

//-----------------------------------------------------------------------------
void RenderArea::drawAreaFilled() const
{
	// Draw 
	glBegin( GL_POLYGON );
	for( int i=0; i < m_poly.nverts(); i++ )
		glVertex2fv( m_poly.vert(i) );
	glEnd();
}

//-----------------------------------------------------------------------------
void RenderArea::render( int gltexid ) const
{
	// TBD
}

//=============================================================================
//  RenderSet
//=============================================================================

//-----------------------------------------------------------------------------
RenderSet::RenderSet()
: m_areaMode( AreaOutline )
{
	float verts[] =
	{
		-.9f, -.9f,
		-.9f,  .9f,
		 .9f,  .9f,
		 .9f, -.9f
	};

	m_areas.clear();
	m_areas.push_back( RenderArea() );
	m_areas[0].polygon().verts() 
		= std::vector<float>( verts, verts+sizeof(verts)/sizeof(float));
}

//-----------------------------------------------------------------------------
int RenderSet::pickVertex( float x, float y )
{
	// Find closest point (brute-force)
	float radius = 0.1f;
	int polygon=-1,
	    vertex=-1;
	float sqdist = std::numeric_limits<float>::max();
	for( int i=0; i < m_areas.size(); i++ )
	{
		for( int j=0; j < m_areas[i].polygon().nverts(); j++ )
		{
			const float* pt = m_areas[i].polygon().vert(j);
			float d2 = (pt[0] - x)*(pt[0] - x) + (pt[1] - y)*(pt[1] - y);
			if( d2 < sqdist )
			{
				sqdist  = d2;
				polygon = i;
				vertex  = j;
			}
		}
	}
	
	if( sqdist < radius*radius )
	{
		m_activeArea   = polygon;
		m_pickedVertex = vertex;
	}
	else
	{
		m_activeArea   = -1;
		m_pickedVertex = -1;
	}
	return polygon;
}

//-----------------------------------------------------------------------------
void RenderSet::getPickedVertexPosition( float& x, float& y ) const
{
	x=0.; y=0.;
	if( m_activeArea < 0 || m_pickedVertex < 0 )
		return;
	
	const float* pt = m_areas[m_activeArea].polygon().vert(m_pickedVertex);
	x = pt[0];
	y = pt[1];
}

//-----------------------------------------------------------------------------
void RenderSet::setPickedVertexPosition( float x, float y )
{
	if( m_activeArea < 0 || m_pickedVertex < 0 )
		return;
	
	float* pt = m_areas[m_activeArea].polygon().vert(m_pickedVertex);
	pt[0] = x;
	pt[1] = y;
}

//-----------------------------------------------------------------------------
void RenderSet::beginRendering() const
{
	// Push the projection matrix and the entire GL state
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	
	// Set 2D ortho projection
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( -1,1, -1,1, -10,10 );
	glMatrixMode( GL_MODELVIEW );	
}

//-----------------------------------------------------------------------------
void RenderSet::endRendering() const
{
	// Pop the projection matrix and GL state back for rendering
	glPopAttrib();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );	
}

//-----------------------------------------------------------------------------
void RenderSet::drawOutline() const
{	
	beginRendering();

	if( m_areaMode == AreaBlackWhite )
	{
		glClearColor( 0,0,0,1 );
		glClear( GL_COLOR_BUFFER_BIT );
	}		
	
	// Draw all areas
	for( int i=0; i < m_areas.size(); i++ )
	{
		if( m_areaMode == AreaOutline )
		{
			if( m_activeArea == i )
			{
				glColor3f( (GLfloat)1,(GLfloat)1,(GLfloat)0 );
				glLineWidth( (GLfloat)3. );
				glPointSize( (GLfloat)12. );
			}
			else
			{
				glColor3f( (GLfloat).7,(GLfloat).7,(GLfloat)0 );
				glLineWidth( (GLfloat)1.5 );
				glPointSize( (GLfloat)6.0 );
			}		
			
			m_areas[i].drawAreaOutline();
		}
		else
		{
			glColor3f( 1,1,1 );
			m_areas[i].drawAreaFilled();
		}
		
	}
	
	endRendering();
}

//-----------------------------------------------------------------------------
void RenderSet::render() const
{
	beginRendering();
	// TBD
	endRendering();
}
