//=============================================================================
//
//  Volume data structures:
//
//  - VolumeDataHeader
//  - VolumeDataAllocator<T>
//  - VolumeDataLoaderRAW<T>
//  - VolumeDataWriterRAW<T>
//  - VolumeData<T>             : public VolumeDataHeader
//  - VolumeDataHeaderLoaderMHD : public VolumeDataHeader
//  - VolumeDataHeaderLoaderDAT : public VolumeDataHeader  (*untested!*)
//  - VolumeAccessor
//
//  TODO:
//  * refactor DataLoader to DataReader
//  * Factory of loaders/readers
//  * forward T* VolumeData<T>::data_ptr() to void* VolumeDataHeader::void_data_ptr()
//
//	Max Hermann, August 14, 2009
//  Last changed: March 24, 2010
//
//=============================================================================
#ifndef __VOLUMEDATA_H
#define __VOLUMEDATA_H

// use std::vector instead of char* as buffer
//#define VOLUME_STD_VECTOR

// default verbosity if none is explicitly specified by the callee 
// (0=no output, 3=debug level output)
#define VOLUMEDATA_DEFAULT_VERBOSITY 0 

#ifdef WIN32
// disable some VisualStudio warnings
#pragma warning(disable: 4244 4018 4267) 
 // warning C4244: '=' : conversion from 'double' to 'T', possible loss of data
 // warning C4018: '<' : signed/unsigned mismatch
 // warning C4267: 'initializing' : conversion from 'size_t' to 'int', possible loss of data
#endif

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cassert>
#include <cmath>
#include <stdexcept>

//-----------------------------------------------------------------------------
//
//	VolumeDataHeader
//
//-----------------------------------------------------------------------------

class VolumeAlignmentPCA;

/// \ingroup VolumeTools
class VolumeDataHeader
{
public:
	friend class VolumeAlignmentPCA;
	
	enum ElementTypeName 
	{ 
		UCHAR  =0,
		CHAR   ,
		USHORT ,
		SHORT  ,
		FLOAT  ,
		DOUBLE ,
		NumberOfElementTypes
	};

	static int   ElementTypeSize  [NumberOfElementTypes];
	static char* ElementTypeString[NumberOfElementTypes];
	
	VolumeDataHeader()
		: m_elementType(UCHAR),
		  m_numChannels(1),
		  m_resX(0), 
		  m_resY(0), 
		  m_resZ(0),
		  m_spacingX(1.), 
		  m_spacingY(1.), 
		  m_spacingZ(1.)
		{}

	virtual ~VolumeDataHeader() {}

	virtual void clear() {}
	
	/// Volume element type
	ElementTypeName elementTypeName() const { return m_elementType; }
	std::string elementTypeNameAsString() const
		{
			return std::string(ElementTypeString[ m_elementType ]);
		}
	
	///@{ Volume resolution
	unsigned int resX() const { return m_resX; }
	unsigned int resY() const { return m_resY; }
	unsigned int resZ() const { return m_resZ; }
	///@}
	
	///@{ Voxel spacing
	double spacingX() const { return m_spacingX; }
	double spacingY() const { return m_spacingY; }
	double spacingZ() const { return m_spacingZ; }
	///@}

	void setSpacing( double dx, double dy, double dz )
	{ m_spacingX = dx;   m_spacingY = dy;   m_spacingZ = dz; }

	/// Number of channels per element
	/// For example a scalar field has 1 channel, a vector field 3 channels.
	unsigned int numChannels() const { return m_numChannels; }
	
	/// Filename of volume data file
	std::string filename() const { return m_filename; }

	///////////////////////////////////////////////////////////////////////////
	// TODO:
	//
	//   Resolve inconsistencies to Raycast version of VolumeData where
	//   similar to the get_size() functions here following functions are
	//   defined:
	//
	//   - size_t elementSize() const  // Size of a single element in bytes
	//   - size_t rawSize() const      // Total raw memory size in bytes
	//
	///////////////////////////////////////////////////////////////////////////

	/// Size in quantity of T
	inline size_t get_size() const
	{
		return m_resX*m_resY*m_resZ*m_numChannels;
	}

	/// Size of (scalar) element type
	inline size_t get_sizeof_element_type()
	{
		return ElementTypeSize[elementTypeName()];
	}

	/// Total size in bytes
	inline size_t get_size_in_bytes() const
	{
		return get_size() * ElementTypeSize[elementTypeName()];
	}

	/// same as VolumeData::get_ofs()*sizeof(T), use for byte-wise addressing
	/// FIXME: Any purpose at all for this func? Addressing through void* 
	///        also requires cast to (T) for all computations!
	inline size_t get_address( int x, int y, int z, int ch )
	{
#ifndef NDEBUG
		assert( x>=0 && x<m_resX && y>=0 && y<m_resY && z>=0 && z<m_resZ &&
		        ch>=0 && ch<m_numChannels );
#endif
		return(  z*m_resX*m_resY*m_numChannels 
		       + y*m_resX*m_numChannels 
		       + x*m_numChannels + ch         ) *ElementTypeSize[m_elementType];
	}
	
protected:
	ElementTypeName m_elementType;
	unsigned        m_numChannels;
	unsigned int    m_resX, m_resY, m_resZ;
	double          m_spacingX, m_spacingY, m_spacingZ;
	std::string m_filename;
};

//-----------------------------------------------------------------------------
//
//	VolumeData
//
//-----------------------------------------------------------------------------

/// Template free access and IO to volumes of different data types.
/// \ingroup VolumeTools
class VolumeDataBase : public VolumeDataHeader
{
public:
	// Implementation in derived template class for specific type
	virtual double get_as_double( int x, int y, int z, int ch=0 ) 
	{ 
		throw std::runtime_error("Fatal error, invalid volume data access!");
	}

	/// Convenience volume loading function
	static VolumeDataBase* load_raw( const char* filename, VolumeDataHeader* mhd );

	virtual void* void_ptr() = 0;
};

template<class T>
class VolumeDataLoaderRAW;

template<class T>
class VolumeDataWriterRAW;

template<class T>
class VolumeDataAllocator;

/// \ingroup VolumeTools
template<class T>
class VolumeData : public VolumeDataBase
{
public:
	friend class VolumeDataLoaderRAW<T>;
	friend class VolumeDataWriterRAW<T>;
	friend class VolumeDataAllocator<T>;

	typedef T ElementType;

	VolumeData()
		: m_buffer(0)
		{}

	/// Free allocated memory but keep header information.
	void clear()
		{
#ifndef VOLUME_STD_VECTOR
			delete [] m_buffer; m_buffer=NULL;
#else
			m_buffer.clear();
#endif
			// Don't reset header information! clear() is intended to free up
			// allocated memory but keep the meta information.
			//*this = VolumeData();  // TEST: does this work as intended?
		}

	void zero()
		{
#ifndef VOLUME_STD_VECTOR
			if( m_buffer )
			{
				memset( m_buffer, 0, get_size()*sizeof(T) );
			}
#else
			// TODO: there sure is a faster way to zero std::vector content
			for( size_t i=0; i < m_buffer.size(); ++i )
				m_buffer[i] = (T)0;
#endif
		}
	
	virtual ~VolumeData()
		{
#ifndef VOLUME_STD_VECTOR
			if( m_buffer ) delete [] m_buffer;
#endif
		}
	
	T operator() ( int x, int y, int z, int ch=0 ) const
		{	
#ifndef VOLUME_STD_VECTOR
			assert( m_buffer );
#endif
			return m_buffer[get_ofs(x,y,z,ch)];
		}

	T& operator() ( int x, int y, int z, int ch=0 )
		{	
#ifndef VOLUME_STD_VECTOR
			assert( m_buffer );
#endif
			return m_buffer[get_ofs(x,y,z,ch)];
		}

	double get_as_double( int x, int y, int z, int ch=0 ) 
	{
		return (double)(*this)(x,y,z,ch);
	}

	/// Returns pointer to volume data buffer (use with care!)
	T* data_ptr()
		{
#ifndef VOLUME_STD_VECTOR
			assert( m_buffer );
#endif
			return &m_buffer[0];
		}

	/// Trilinear interpolation (TODO: handle multiple channels simulatenously)
	T interp( double x, double y, double z, int ch=0 ) const;

protected:
	void* void_ptr()
	{
		return (void*)data_ptr();
	}

	/// Offset function reflects memory layout
	inline int get_ofs( int x, int y, int z, int ch ) const
	{
		assert( x>=0 && x<m_resX && y>=0 && y<m_resY && z>=0 && z<m_resZ &&
		        ch>=0 && ch<m_numChannels );

		return z*m_resX*m_resY*m_numChannels 
		     + y*m_resX*m_numChannels 
		     + x*m_numChannels + ch;
	}

private:
#ifdef VOLUME_STD_VECTOR
	std::vector<T> m_buffer;
#else
	T* m_buffer;
#endif
};

//-----------------------------------------------------------------------------
//
//	VolumeDataAllocator
//
//-----------------------------------------------------------------------------

//template<class T1, class T2>
//class VolumeDataConverter
//{
//public:
//	VolumeData<T2>* convert( VolumeData<T1>* vol, float scale, float shift );
//};

/// \ingroup VolumeTools
template<class T>
class VolumeDataAllocator
{
public:
	/// Specialized constructor to set element type according to template type
	VolumeDataAllocator();

	/// Allocate volume of the given size.
	/// \todo Let the user specify additional header information like spacing.
	VolumeData<T>* allocate( int resX, int resY, int resZ, int numChannels,
                         double spacingX, double spacingY, double spacingZ );

	// Convenience functions with default parameters for channels and/or spacing
	VolumeData<T>* allocate( int resX, int resY, int resZ )
	{
		return allocate( resX,resY,resZ, 1, 1.0,1.0,1.0 );
	}

	VolumeData<T>* allocate( int resX, int resY, int resZ, int numChannels )
	{
		return allocate( resX,resY,resZ, numChannels, 1.0,1.0,1.0 );
	}

	VolumeData<T>* allocate( int resX, int resY, int resZ, 
                         double spacingX, double spacingY, double spacingZ )
	{
		return allocate( resX,resY,resZ, 1, spacingX,spacingY,spacingZ );
	}

protected:
	VolumeDataHeader::ElementTypeName m_elementType;
};

//-----------------------------------------------------------------------------
//
//	VolumeDataLoaderRAW
//
//-----------------------------------------------------------------------------
/// \ingroup VolumeTools
template<class T>
class VolumeDataLoaderRAW
{
public:	
	static T* loadRAW( const char* filename, int& bytesRead, 
		               int verbosity=VOLUMEDATA_DEFAULT_VERBOSITY );

	VolumeDataLoaderRAW()
		: m_gcount(0)
	{}

	/// Load volume data from uncompressed raw file
	/// @param filename filename of raw data file
	/// @param header meta image header object (usually read from a MHD file)
	/// @param verbosity output verbosity (0=no output, 3=debug level output)
	VolumeData<T>* load( const char* filename, VolumeDataHeader* header,
		                 int verbosity=VOLUMEDATA_DEFAULT_VERBOSITY );

	/// Return number of bytes read on last call to load().
	int gcount() const { return m_gcount; }

private:
	int m_gcount;
};

//-----------------------------------------------------------------------------
//
//	VolumeDataWriterRAW
//
//-----------------------------------------------------------------------------
/// \ingroup VolumeTools
template<class T>
class VolumeDataWriterRAW
{
public:	
	static void writeRAW( const char* filename, const VolumeData<T>* volume,
		                  int verbosity=VOLUMEDATA_DEFAULT_VERBOSITY );

	static void writeMHD( const char* filename, const VolumeData<T>* volume,
		                  int verbosity=VOLUMEDATA_DEFAULT_VERBOSITY );

	/// Write volume data to uncompressed RAW file with accompanying MHD file.
	/// @param name filename of raw data output file (extension ignored)
	/// @param volume volume data object (with correctly set header infos)
	/// @param verbosity output verbosity (0=no output, 3=debug level output)
	void write( const char* name,
	            const VolumeData<T>* volume,
		        int verbosity=VOLUMEDATA_DEFAULT_VERBOSITY );
};

//-----------------------------------------------------------------------------
//
//	VolumeDataHeaderLoaderMHD
//
//-----------------------------------------------------------------------------
/// \ingroup VolumeTools
class VolumeDataHeaderLoaderMHD : public VolumeDataHeader
{
public:
	VolumeDataHeaderLoaderMHD() {}
	VolumeDataHeaderLoaderMHD( const VolumeDataHeader& super )
		: VolumeDataHeader(super)
		{}
	bool load( const char* filename );
	void save( const char* filename );
	void setDataFilename( std::string fname ) { m_filename = fname; }
};

//-----------------------------------------------------------------------------
//
//	VolumeDataHeaderLoaderDAT
//
//-----------------------------------------------------------------------------
/// \ingroup VolumeTools
class VolumeDataHeaderLoaderDAT : public VolumeDataHeader
{
public:
	VolumeDataHeaderLoaderDAT() {}
	VolumeDataHeaderLoaderDAT( const VolumeDataHeader& super )
		: VolumeDataHeader(super)
		{}
	bool load( const char* filename );
	//void save( const char* filename ); // TBD
	void setDataFilename( std::string fname ) { m_filename = fname; }
};

//-----------------------------------------------------------------------------
//
//	VolumeAccessor
//
//-----------------------------------------------------------------------------

/// Accessor for volume data which supports dynamic voxel operations
/// like tresholding. Dynamic means here that the volume data is not changed
/// but the specified operations/filters are applied online when a voxel
/// is accessed via operator().
///
/// \todo currently restricted to scalar volumes, extend to multiple channels
/// 
/// \ingroup VolumeTools
template<class T>
class VolumeAccessor : public VolumeDataHeader
{
public:
	/// Methods to handle out-of-bounds indexing
	enum IndexMode { 
		NoIndexCheck,  ///< perform no range checking at all
		ClampIndex,    ///< clamp index range to volume resolution
		ZeroIndex      ///< return zero intensity outside index range
//		MirrorIndex,   ///< mirror index range
	};

	VolumeAccessor()
		: m_vol(0), m_threshold(false), m_indexMode(NoIndexCheck)
		{}

	void setVolumeData( VolumeData<T>* vol );

	float operator() ( int x, int y, int z ) const;

	/// Apply simple threshold excluding voxel intensities outside.
	/// Excluded voxels are set to zero.
	/// Threshold filter is automatically enabled.
	void setThreshold( T lower, T upper );
	void setThresholdEnabled( bool enable );
	void setIndexMode( IndexMode mode );

	/// Flag thresholded is true iff last voxel accessed was thresholded
	bool thresholdedFlag;

private:
	VolumeData<T>* m_vol;
	bool m_threshold;
	T m_threshLo, m_threshUp;
	IndexMode m_indexMode;
};


//=============================================================================
//
//	Template implementations
//
//=============================================================================

template<class T>
void VolumeDataWriterRAW<T>::write( const char* fname,
                                    const VolumeData<T>* volume,
                                    int verbosity                )
{
	if( !volume )
		return;

	using namespace std;
	string filename( fname ), path, name;

	// replace backslashes (indicating directory structure) by slashes
	for( unsigned int i=0; i < filename.length(); ++i )
		if( filename[i] == '\\' )  filename[i] = '/';					
	
	// extract path and name
	path = "";
	size_t seploc;
	seploc = filename.find_last_of("/");
	if( seploc != string::npos )
		path = filename.substr(0,seploc+1);
	else
		seploc = -1;
	size_t ptloc = filename.find_last_of(".");
	name = filename.substr( seploc+1, ptloc-(seploc+1) );

	string filename_mhd = path + name + ".mhd";
	string filename_raw = path + name + ".raw";

	// write MHD
	if( verbosity > 2 ) cout << "Saving MHD file '" << filename_mhd << "'..." << endl;
	VolumeDataHeader mhd = (VolumeDataHeader)*volume;
	VolumeDataHeaderLoaderMHD ml( mhd );
	ml.setDataFilename( name + string(".raw") );
	ml.save( filename_mhd.c_str() );

	// write RAW
	if( verbosity > 2 ) cout << "Saving RAW file '" << filename_raw << "'..." << endl;
	writeRAW( filename_raw.c_str(), volume, verbosity );
}

template<class T>
void VolumeDataWriterRAW<T>::writeRAW( const char* filename,
                                       const VolumeData<T>* volume,
                                       int verbosity                )
{
	using namespace std;

	ofstream f;
	if( verbosity > 2 ) cout << "Opening file " << filename 
		                     << " for writing " << endl;

	// open file
	f.open( filename, ios::out | ios::binary );
	if( !f.is_open() )
	{
		cerr << "Error: Couldn't open file " << filename << "!" << endl;
	}
	
	// write data as one block
	size_t size = volume->get_size() * sizeof(T);
	f.write( (char*)volume->m_buffer, size );

	f.close();
}

template<class T>
T* VolumeDataLoaderRAW<T>::loadRAW( const char* filename, 
                                    int& bytesRead,
									int verbosity         )
{
	using namespace std;
	ifstream f;

	if( verbosity > 2 ) cout << "Opening file " << filename << endl;

	// open file
	f.open( filename, ios::in | ios::binary );	
	if( !f.is_open() )
	{
		cerr << "Error: Couldn't open file " << filename << "!" << endl;
		return 0;
	}

	// get file size
	f.seekg( 0, ios::end );
	int size = (int)f.tellg();
	f.seekg( 0, ios::beg );

	if( verbosity > 2 ) cout << "Allocate " << size/(1024*1024) << " MB..." << endl;
	
	// allocate buffer
	T* buffer = 0;
	try
	{
		buffer = new T[ size / sizeof(T) ];	
	}
	catch( bad_alloc& )
	{
		cerr << "Error allocating " << size/(1024*1024) << "MB of memory!" << endl;
		return 0;
	}
	
	if( verbosity > 2 ) cout << "Buffer address=0x" << std::hex << (size_t)buffer << std::dec << endl;	
	if( verbosity > 2 ) cout << "Read data as one block..."; cout.flush();

	// read data as block
	f.read( (char*)buffer, size );
	bytesRead = (int)f.gcount();

	if( verbosity > 2 ) cout << "done, " << bytesRead << " bytes read." << endl;
	if( verbosity > 2 ) cout << "Closing file" << endl;

	// close file
	f.close();
	return buffer;
}

template<class T>
VolumeData<T>* VolumeDataLoaderRAW<T>::load( const char* filename, 
											 VolumeDataHeader* header,
											 int verbosity             )
{
	if( verbosity > 2 ) std::cout << "Creating new VolumeData object" << std::endl;
	VolumeData<T>* volume = new VolumeData<T>();
	
	// set header part (resolution, spacing, etc.)
	*(dynamic_cast<VolumeDataHeader*>( volume )) = *header;
	
	// load raw volume data
	T* buffer = loadRAW( filename, m_gcount, verbosity );
	if( !buffer )
	{			
		delete volume; volume = 0;
		return 0;
	}

	// check size
	int size = volume->resX()*volume->resY()*volume->resZ() * sizeof(T) * volume->numChannels();
	if( size != m_gcount )
	{
		std::cerr << "Error: Size mismatch between volume dimensions described "
			      << "in header file and RAW volume data size!" << std::endl;
		delete volume; volume = 0;
		return 0;
	}

#ifdef VOLUME_STD_VECTOR
std::cout << "memcpy" << std::endl;	
	volume->m_buffer.resize( size );
	memcpy( &(volume->m_buffer[0]), buffer, size );
#else
	volume->m_buffer = buffer;
	if( verbosity > 2 ) std::cout << "Volume buffer address = 0x" << std::hex << (size_t)(volume->m_buffer) << std::dec << std::endl;
#endif
	return volume;
}	


template<class T>
void VolumeAccessor<T>::setVolumeData( VolumeData<T>* vol )
{
	// copy header information (like resolution and spacing)
	if( vol )
		*((VolumeDataHeader*)this) = *(VolumeDataHeader*)vol;
	m_vol = vol;
}

template<class T>
void VolumeAccessor<T>::setThreshold( T lower, T upper )
{
	m_threshLo = lower;
	m_threshUp = upper;
	m_threshold = true;
}

template<class T>
void VolumeAccessor<T>::setThresholdEnabled( bool enable )
{
	m_threshold = enable;
}

template<class T>
void VolumeAccessor<T>::setIndexMode( IndexMode mode )
{
	m_indexMode = mode;
}

template<class T>
float VolumeAccessor<T>::operator() ( int x, int y, int z ) const
{
	// although this function is const, it is allowed to change flags
	bool& thresholdedFlag = const_cast<bool&>( this->thresholdedFlag );

	if( !m_vol ) 
	{
		std::cerr << "Warning: Trying to access NULL volume!" << std::endl;
		return 0.;
	}

	// reset flags
	thresholdedFlag = false;

	// adjust indices
	switch( m_indexMode )
	{
		default:
		case NoIndexCheck:
			break;
		case ClampIndex:
			// clamp to volume borders
			if( x < 0 ) x = 0;  else  if( x > (int)resX()-1 )  x = resX()-1;
			if( y < 0 ) y = 0;  else  if( y > (int)resY()-1 )  y = resY()-1;
			if( z < 0 ) z = 0;  else  if( z > (int)resZ()-1 )  z = resZ()-1;
			break;
		case ZeroIndex:
			// return zero if any coordinate is outside volume
			if( (x < 0) || (x >= (int)resX()) ||
				(y < 0) || (y >= (int)resY()) ||
				(z < 0) || (z >= (int)resZ())    )  return 0.f;
			break;
	}

	// get voxel intensity
	T v = (*m_vol)(x,y,z);

	// apply threshold
	if( m_threshold )
	{		
		thresholdedFlag = !((v >= m_threshLo) && (v <= m_threshUp));
		v = thresholdedFlag ? T(0) : v;
	}

	return (float)v;
}

template<class T>
VolumeData<T>* VolumeDataAllocator<T>
	::allocate( int resX, int resY, int resZ, int numChannels, 
	            double spacingX, double spacingY, double spacingZ )
{
	using namespace std;

	int verbosity = 0;

	if( verbosity > 2 ) std::cout << "Creating new VolumeData object" << std::endl;
	VolumeData<T>* volume = new VolumeData<T>();
	
	// set header part (resolution, spacing, etc.)
	volume->m_numChannels = numChannels;
	volume->m_resX = resX;
	volume->m_resY = resY;
	volume->m_resZ = resZ;
	volume->m_spacingX = 1;
	volume->m_spacingY = 1;
	volume->m_spacingZ = 1;
	volume->m_filename = "";
	volume->m_elementType = m_elementType;
	// m_elementType is set in specialized constructor
	
	// allocate buffer
	size_t size = volume->get_size() * sizeof(T);
	T* buffer = 0;
	try
	{
		buffer = new T[ size / sizeof(T) ];
	}
	catch( std::bad_alloc& )
	{
		cerr << "Error allocating " << size/(1024*1024) << "MB of memory!" << endl;
		return 0;
	}

#ifdef VOLUME_STD_VECTOR
std::cout << "memcpy" << std::endl;	
	volume->m_buffer.resize( size );
	memcpy( &(volume->m_buffer[0]), buffer, size*sizeof(T) );
#else
	volume->m_buffer = buffer;
	if( verbosity > 2 ) std::cout << "Volume buffer address = 0x" << std::hex << (int)(volume->m_buffer) << std::dec << std::endl;
#endif
	return volume;
}

template<class T>
T VolumeData<T>::interp( double x, double y, double z, int ch ) const
{
	using std::floor;

	// trilinear interpolation
	// slightly simplified since voxel size is canonical (1,1,1)
	T f;

	// voxel position
	int x0 = (int)floor(x),
		y0 = (int)floor(y),
		z0 = (int)floor(z);

	// boundary conditions
	int x1 = x0 + 1,
		y1 = y0 + 1,
		z1 = z0 + 1;
	int  condx = (int)(x1 >= (int)m_resX),
		 condy = (int)(y1 >= (int)m_resY),
		 condz = (int)(z1 >= (int)m_resZ);
	if( (condx + condy + condz) == 0 )
	{
		// A) trilinear interpolation

		// weights 
		double a = x - x0,
			   b = y - y0,
			   c = z - z0;

		// evaluate corners
		T f000 = (*this)(x0,y0,z0,ch), f100 = (*this)(x1,y0,z0,ch),
		  f010 = (*this)(x0,y1,z0,ch), f110 = (*this)(x1,y1,z0,ch),
		  f001 = (*this)(x0,y0,z1,ch), f101 = (*this)(x0,y1,z0,ch),
		  f011 = (*this)(x0,y1,z1,ch), f111 = (*this)(x1,y1,z1,ch);

		// interpolate
		f = f000*((1-a)*(1-b)*(1-c)) +   f100*(a *(1-b)*(1-c))  +
		    f010*((1-a)*   b *(1-c)) +   f110*(a *   b *(1-c))  +
		    f001*((1-a)*(1-b)*   c ) +   f101*(a *(1-b)*   c )  +
		    f011*((1-a)*   b *   c ) +   f111*(a *   b *   c );
	}
	else
	if( (condx + condy + condz) == 1 )
	{
		// B) bilinear interpolation

		T f00, f10, f01, f11;
		double a,b;

		f00 = (*this)(x0,y0,z0,ch);

		if( condx ) //  along y-z
		{
			                            f10 = (*this)(x0,y1,z0,ch);
			f11 = (*this)(x0,y1,z1,ch); f01 = (*this)(x0,y0,z1,ch); 
			a = y - y0;
			b = z - z0;
		}
		else
		if( condy ) //  along x-z
		{
			                            f10 = (*this)(x1,y0,z0,ch);
			f11 = (*this)(x1,y0,z1,ch); f01 = (*this)(x0,y0,z1,ch); 
			a = x - x0;
			b = z - z0;
		}
		else
		if( condz ) //  along x-y
		{
			                            f10 = (*this)(x1,y0,z0,ch);
			f11 = (*this)(x1,y1,z0,ch); f01 = (*this)(x0,y1,z0,ch); 
			a = x - x0;
			b = y - y0;
		}
		else
			assert(false);	// should never be reached

		// interpolate
		f = f00*((1-a)*(1-b)) + f10*(a*(1-b)) + f01*((1-a)*b) + f11*(a*b);
	}
	else
	if( (condx + condy + condz) == 2 )
	{
		// C) linear interpolation
		T f0, f1;
		double a;
		f0 = (*this)(x0,y0,z0,ch);

		if( !condx )
		{
			f1 = (*this)(x1,y0,z0,ch);
			a = x - x0;
		}
		else 
		if( !condy )
		{
			f1 = (*this)(x0,y1,z0,ch);
			a = y - y0;
		}
		else
		if( !condz )
		{
			f1 = (*this)(x0,y0,z1,ch);
			a = z - z0;
		}
		else
			assert(false);  // should never be reached

		// interpolate
		f = f0*(1-a) + f1*a;
	}
	else
		// D) corner voxel
		f = (*this)(x0,y0,z0,ch);


	return f;
}



#endif // __VOLUMEDATA_H
