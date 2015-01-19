#include "RenderSet.h"
#include "ProjectMe.h" // RenderSet depends on ProjectMe
#include "glbase.h"
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
using std::cout;
using std::cerr;
using std::endl;
using std::string;

//=============================================================================
//  Serializable
//=============================================================================

//-----------------------------------------------------------------------------
void Serializable::serializeToDisk( string filename )
{
	PropertyTree pt = serialize();
	write_xml( filename, pt );
}

//-----------------------------------------------------------------------------
bool Serializable::deserializeFromDisk( string filename )
{
	PropertyTree pt;
	read_xml( filename, pt );
	deserialize( pt );
	return true; // FIXME: Implement some error-checking!
}

//=============================================================================
//  ModuleBase
//=============================================================================

std::map<std::string,int> ModuleBase::s_typeCount;

//-----------------------------------------------------------------------------
Serializable::PropertyTree& ModuleBase::serialize() const
{
	static Serializable::PropertyTree cache;
	cache.clear();
	cache.put("ModuleBase.Type",getModuleType());
	cache.put("ModuleBase.Name",getName());

	// Write parameters and options
	parameters().write( cache );
	options().write( cache );

	return cache;
}

//-----------------------------------------------------------------------------
void ModuleBase::deserialize( Serializable::PropertyTree& pt )
{
	m_moduleTypeName = pt.get("ModuleBase.Type",string("<Unknown type>"));
	setName( pt.get("ModuleBase.Name", getDefaultName()) );

	// Read parameters and options
	parameters().read( pt );
	options().read( pt );
}

//-----------------------------------------------------------------------------
std::string ModuleBase::getDefaultName()
{
	std::stringstream ss;
	if( !s_typeCount.count( getModuleType() ) )
		ss << Serializable::getDefaultName();
	else
		ss << getModuleType() << s_typeCount[getModuleType()];
	return ss.str();
}


//=============================================================================
//  ModuleRenderer
//=============================================================================

//-----------------------------------------------------------------------------
Serializable::PropertyTree& ModuleRenderer::serialize() const
{
	static Serializable::PropertyTree cache;
	cache = ModuleBase::serialize();
	cache.put("ModuleRenderer.Position.x",m_position.x);
	cache.put("ModuleRenderer.Position.y",m_position.y);
	return cache;
}

//-----------------------------------------------------------------------------
void ModuleRenderer::deserialize( Serializable::PropertyTree& pt )
{
	ModuleBase::deserialize( pt );

	float
		x = pt.get("ModuleRenderer.Position.x",0.f),
		y = pt.get("ModuleRenderer.Position.y",0.f);

	setPosition( Position(x,y) );
}

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

	// DEBUG: Render estimated center point for projective mapping
#if 1
	if( isQuad() )
	{
		// FIXME: Update only on geometry change!
		m_hq.compute( m_poly.vert(0), m_poly.vert(1), m_poly.vert(2), m_poly.vert(3) );

		glBegin( GL_LINES );
		glVertex2fv( m_poly.vert(0) );	glVertex2fv( m_poly.vert(2) );
		glVertex2fv( m_poly.vert(1) );	glVertex2fv( m_poly.vert(3) );
		glEnd();

		glBegin( GL_POINTS );
		glVertex2fv( m_hq.getIntersection() );
		glEnd();
	}
#endif
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
	// Apply projective mapping for quads
	bool projmap = isQuad();
	if( projmap ) // FIXME: Update only on geometry change!
		m_hq.compute( m_poly.vert(0), m_poly.vert(1), m_poly.vert(2), m_poly.vert(3) );

	// Render polygon in OpenGL immediate mode
	glBindTexture( GL_TEXTURE_2D, gltexid );
	glBegin( (m_poly.nverts()==4) ? GL_QUADS : GL_POLYGON );
	for( int i=0; i < m_poly.nverts(); i++ )
	{
		if( projmap )
		{
			// Projective mapping using GL homogeneous 4th texture coordinate
			const float* q = m_hq.getHomogeneousWeights();
			glTexCoord4f( q[i]*m_poly.texcoord(i)[0], q[i]*m_poly.texcoord(i)[1], 
				0.f, q[i] );
		}
		else
			glTexCoord2fv( m_poly.texcoord(i) );
		glVertex2fv  ( m_poly.vert(i) );
	}
	glEnd();
}

//-----------------------------------------------------------------------------
Serializable::PropertyTree& RenderArea::serialize() const
{
	static Serializable::PropertyTree cache;
	cache.clear();
	
	cache.put( "Name"        , getName() );
	cache.put( "NumVertices" , m_poly.nverts() );

	for( int i=0; i < m_poly.nverts(); i++ )
	{		
		Serializable::PropertyTree t;
		t.put( "Pos.x", m_poly.vert(i)[0] );
		t.put( "Pos.y", m_poly.vert(i)[1] );
		t.put( "TexCoord.u", m_poly.texcoord(i)[0] );
		t.put( "TexCoord.v", m_poly.texcoord(i)[1] );

		cache.add_child( "Vertices.Vertex", t );
	}

	return cache;
}

//-----------------------------------------------------------------------------
void RenderArea::deserialize( Serializable::PropertyTree& pt )
{
	setName( pt.get( "Name", getDefaultName() ) );
	int nverts = pt.get( "NumVertices", -1 ); // unused

	m_poly.clear();

	BOOST_FOREACH( PropertyTree::value_type& l, pt.get_child("Vertices") )
	{		
		if( l.first.compare("Vertex")==0 )
		{
			float x = l.second.get<float>("Pos.x"),
			      y = l.second.get<float>("Pos.y"),
				  u = l.second.get<float>("TexCoord.u"),
				  v = l.second.get<float>("TexCoord.v");

			m_poly.verts().push_back( x );
			m_poly.verts().push_back( y );
			m_poly.texcoords().push_back( u );
			m_poly.texcoords().push_back( v );
		}
		else
			cerr << "RenderArea::deserialize() : Unknown key "
			  "\"" << l.first << "\" in Vertices!" << endl;
	}

	if( m_poly.nverts() != nverts )
		cerr << "RenderArea::deserialize() : Mismatching number of vertices!" << endl;
}

//=============================================================================
//  RenderSet
//=============================================================================

//-----------------------------------------------------------------------------
RenderSet::RenderSet()
: ModuleRenderer("RenderSet"),
  m_areaMode( AreaOutline ),
  m_moduleManager( NULL ),
  m_projectMe( NULL )
{
	addArea( RenderArea(), 0 );
	setName("RenderSet");
}

//-----------------------------------------------------------------------------
void RenderSet::addArea( RenderArea area, ModuleRenderer* module )
{
	m_areas   .push_back( area );
	m_mapper  .push_back( module );
	
	// WORKAROUND: Add enough channels to match number of areas
	while( m_areas.size() > m_channels.size() )
		m_channels.push_back( 0 );

	m_channels.back() = module ? module->target() : -1;
}

//-----------------------------------------------------------------------------
void RenderSet::setModule( int areaIdx, ModuleRenderer* module )
{
	if( areaIdx >= 0 && areaIdx < m_areas.size() )
	{
		m_mapper[areaIdx]   = module;
		m_channels[areaIdx] = module ? module->target() : -1;
			// WORKAROUND: Synchronize channels with mapper
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
	float radius = 0.3f;
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
void RenderSet::render_internal( int texid ) /*const*/
{
	beginRendering();

	// Set opaque black background
	glClearColor( 0.f, 0.f, 0.f, 1.f );
	glClear( GL_COLOR_BUFFER_BIT );

	// Enable texturing
	glColor4f( 1.f, 1.f, 1.f, 1.f );
	glEnable( GL_TEXTURE_2D );

	// Disable z-buffer
	glDisable( GL_DEPTH_TEST );

	// Alpha-blending (classical transparency, no pre-multiplied alpha)
	glBlendFunc( GL_FUNC_ADD, GL_FUNC_ADD );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );

	for( int i=0; i < m_areas.size(); i++ )
	{
		if( texid < 0 )
		{
			// Channel overrides mapper!
			if( m_channels[i] >= 0 )
			{
				m_areas[i].render( m_channels[i] );
			}
			// Default behaviour: Render module textures onto render areas
			else if( m_mapper[i] ) 	
			{
				m_areas[i].render( m_mapper[i]->target() );
			}
		}
		else
		{
			// Debug: Render given texture id onto all render areas
			m_areas[i].render( texid );
		}
	}

	glDisable( GL_BLEND );
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	endRendering();
}

//-----------------------------------------------------------------------------
Serializable::PropertyTree& RenderSet::serialize() const
{
	static Serializable::PropertyTree cache;
	cache = ModuleRenderer::serialize();
	
	cache.put("RenderSet.NumAreas",m_areas.size());

	// Write render area configuration
	for( int i=0; i < m_areas.size(); i++ )
		cache.add_child( "RenderSet.RenderArea", m_areas[i].serialize() );

	// Write mapping area to module
	for( int i=0; i < m_mapper.size(); i++ )
	{
		Serializable::PropertyTree t;
		ModuleRenderer* mod = m_mapper[i];
		t.put( "AreaIndex", i );
		if( mod )
		{
			// Use module name to later re-establish mapping on deserialization
			t.put( "ModuleName", mod->getName() );
			t.put( "ModuleType", mod->getModuleType() );
		}
		else
		{
			// Use empty strings to indicate non-set mapping
			t.put( "ModuleName", "" );
			t.put( "ModuleType", "" );
		}

		cache.add_child( "RenderSet.ModuleMapper", t );
	}

	return cache;
}

//-----------------------------------------------------------------------------
void RenderSet::deserialize( Serializable::PropertyTree& pt )
{
	// WORKAROUND: Pointer to already deserialized module manager is required!
	if( !m_moduleManager )
	{
		cerr << "RenderSet::deserialize() : Pointer to module manager not set!" << endl;
		return;
	}

	int numAreas = pt.get<int>("RenderSet.NumAreas",0);

	clear();
	m_mapper  .resize( numAreas, NULL ); // Mappings can be deserialized before areas
	m_channels.resize( numAreas );
	
	BOOST_FOREACH( PropertyTree::value_type& v, pt.get_child("RenderSet") )
	{		
		// Read render area configuration
		if( v.first.compare("RenderArea")==0 )
		{
			RenderArea area;
			area.deserialize( v.second );
			addArea( area, NULL );
		}
		else
		// Read mapping area to module
		if( v.first.compare("ModuleMapper")==0 )
		{
			int    idx  = v.second.get<int>("AreaIndex");
			string name = v.second.get("ModuleName",string("")),
			       type = v.second.get("ModuleType",string(""));

			if( !name.empty() ) // Empty name indicates no mapping
			{
				ModuleRenderer* mod =
					m_moduleManager->findModule( name, type );
				if( mod && (idx>=0) && (idx<m_mapper.size()) )
				{
					// Set mapping
					m_mapper[idx] = mod;
					m_channels[idx] = mod ? mod->target() : -1;
				}
				else
				{
					if( !mod )
						cerr << "RenderSet::deserialize() : No module with name=\""<<name<<"\" of type \""<<type<<"\" found!" << endl;
					else
						cerr << "RenderSet::deserialize() : Invalid mapping index " << idx << "!" << endl;
				}
			}
		}
	}

	if( m_areas.size() != numAreas )
		cerr << "RenderSet::deserialize() : Mismatching number of areas!" << endl;	
}

//-----------------------------------------------------------------------------
void RenderSet::setChannel( int idx, int texId )
{
	if( idx>=0 && idx<m_channels.size() )
	{
		m_channels[idx] = texId;

		// WORKAROUND: Synchronize module mapper with channels
		if( m_projectMe )
			setModule( idx, m_projectMe->moduleFromTarget( texId ) );
	}
}

int  RenderSet::channel( int idx ) const
{
	if( idx>=0 && idx<m_channels.size() )
		return m_channels[idx];
	return -1;
}

int  RenderSet::numChannels() const
{
	return (int)m_channels.size();
}
