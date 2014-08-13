#include "RenderSet.h"
#include "glbase.h"
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

//=============================================================================
//  RenderArea
//=============================================================================

//-----------------------------------------------------------------------------
RenderArea::RenderArea()
{
	float verts[] =
	{
		-.9f, -.9f,
		-.9f,  .9f,
		 .9f,  .9f,
		 .9f, -.9f
	};

	float texcoords[] =
	{
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f
	};

	polygon().clear();
	polygon().verts() 
		= std::vector<float>( verts, verts+sizeof(verts)/sizeof(float));
	polygon().texcoords()
		= std::vector<float>( texcoords, texcoords+sizeof(texcoords)/sizeof(float));
}

//-----------------------------------------------------------------------------
RenderArea::RenderArea( float xmin, float ymin, float xmax, float ymax )
{
	polygon().clear();
	polygon().verts().push_back( xmin );  polygon().verts().push_back( ymin );
	polygon().verts().push_back( xmin );  polygon().verts().push_back( ymax );
	polygon().verts().push_back( xmax );  polygon().verts().push_back( ymax );
	polygon().verts().push_back( xmax );  polygon().verts().push_back( ymin );

	float texcoords[] =
	{
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f
	};
	polygon().texcoords()
		= std::vector<float>( texcoords, texcoords+sizeof(texcoords)/sizeof(float));
}

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
	glBindTexture( GL_TEXTURE_2D, gltexid );
	glBegin( (m_poly.nverts()==4) ? GL_QUADS : GL_POLYGON );
	for( int i=0; i < m_poly.nverts(); i++ )
	{
		glTexCoord2fv( m_poly.texcoord(i) );
		glVertex2fv  ( m_poly.vert(i) );
	}
	glEnd();	
}

//=============================================================================
//  RenderSet
//=============================================================================

//-----------------------------------------------------------------------------
RenderSet::RenderSet()
: m_areaMode( AreaOutline )
{
	addArea( RenderArea(), 0 );
}

//-----------------------------------------------------------------------------
void RenderSet::addArea( RenderArea area, ModuleRenderer* module )
{
	m_areas .push_back( area );
	m_mapper.push_back( module );
}

//-----------------------------------------------------------------------------
void RenderSet::setModule( int areaIdx, ModuleRenderer* module )
{
	if( areaIdx >= 0 && areaIdx < m_areas.size() )
	{
		m_mapper[areaIdx] = module;
	}
	else
	{
		cerr << "RenderSet::setModule() : Called with invalid index "
		     << areaIdx << "!" << endl;		     
	}
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
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
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
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );	
	glPopAttrib();
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
void RenderSet::render( int texid ) const
{
	beginRendering();

	glColor4f( 1.f, 1.f, 1.f, 1.f );
	glEnable( GL_TEXTURE_2D );

	for( int i=0; i < m_areas.size(); i++ )
	{
		if( texid < 0 )
		{
			// Default behaviour: Render module textures onto render areas
			if( !m_mapper[i] ) continue;
			texid = m_mapper[i]->target();
			m_areas[i].render( texid );
		}
		else
		{
			// Debug: Render given texture id onto all render areas
			m_areas[i].render( texid );
		}
	}

	glDisable( GL_TEXTURE_2D );
	endRendering();
}

//-----------------------------------------------------------------------------
void RenderSet::serialize( Serializable::PropertyTree& pt ) const
{
	/*
	Serializable::PropertyTree root; 
	//node.put("Name",m_name);
	root.put("RenderSet.NumAreas",m_areas.size());
	for( int i=0; i < m_areas.size(); i++ )
	{
		Serializable::PropertyTree node;
		
		m_areas[i].serialize( pt_area );

		pt.push_back( s
	}*/
}

//-----------------------------------------------------------------------------
void RenderSet::deserialize( Serializable::PropertyTree& pt )
{
}

