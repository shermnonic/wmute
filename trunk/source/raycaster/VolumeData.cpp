//	Max Hermann, August 14, 2009
#include "VolumeData.h"
#include <sstream>
#include <cmath>   // for: fabs()
#include <cstdlib> // for: atoi(), atof(), HUGE_VAL, INT_MAX, INT_MIN

using namespace std;

#ifdef VOLUMEDATA_DEBUG_HEADERLOADERS
#define VOLUMEDATA_DEBUG_HEAD_PRINT( M ) cout << M;
#else
#define VOLUMEDATA_DEBUG_HEAD_PRINT( M )
#endif

template<> VolumeDataAllocator<unsigned char>::VolumeDataAllocator()
{ m_elementType = VolumeDataHeader::UCHAR; }
template<> VolumeDataAllocator<char>::VolumeDataAllocator()
{ m_elementType = VolumeDataHeader::CHAR; }
template<> VolumeDataAllocator<unsigned short>::VolumeDataAllocator()
{ m_elementType = VolumeDataHeader::USHORT; }
template<> VolumeDataAllocator<short>::VolumeDataAllocator()
{ m_elementType = VolumeDataHeader::SHORT; }
template<> VolumeDataAllocator<float>::VolumeDataAllocator()
{ m_elementType = VolumeDataHeader::FLOAT; }
template<> VolumeDataAllocator<double>::VolumeDataAllocator()
{ m_elementType = VolumeDataHeader::DOUBLE; }


// static members
int   VolumeDataHeader::ElementTypeSize  [VolumeDataHeader::NumberOfElementTypes] 
	= { 
		1, // UCHAR
		1, // CHAR
		2, // USHORT
		2, // SHORT
		4, // FLOAT
		8  // DOUBLE
	  };
char* VolumeDataHeader::ElementTypeString[VolumeDataHeader::NumberOfElementTypes]
	= { "UCHAR",
		"CHAR",
		"USHORT",
		"SHORT",
		"FLOAT",
		"DOUBLE"
	  };

//-----------------------------------------------------------------------------
//
//	VolumeDataHeaderLoaderMHD
//
//-----------------------------------------------------------------------------

void VolumeDataHeaderLoaderMHD::save( const char* filename )
{
	ofstream of;
	of.open( filename );
	if( !of.is_open() )
	{
		cerr << "Error: Couldn't save MHD file '" << filename << "'!" << endl;
		return;
	}

	// MHD type string
	bool validType = ((m_elementType>=UCHAR)&&(m_elementType<NumberOfElementTypes));
	assert( validType );
	string mhd_type_string = string("MET_") + string(ElementTypeString[m_elementType]);

	of << "NDims                   = 3" << endl;
	of << "ElementType             = " << mhd_type_string << endl;
	of << "DimSize                 = " << m_resX << " " << m_resY << " " << m_resZ << endl;
	of << "ElementNumberOfChannels = " << m_numChannels << endl;
	of << "ElementSpacing          = " << m_spacingX << " " << m_spacingY << " " << m_spacingZ << endl;
	of << "ElementDataFile         = " << m_filename << endl;

	of.close();
}

bool VolumeDataHeaderLoaderMHD::load( const char* filename )
{
	int    resX, resY, resZ;
	int    numChannels = 1;	// optional, defaults to 1 channel
	double spacingX, spacingY, spacingZ;
	string objectFilename;
	VolumeDataHeader::ElementTypeName elementType;

	bool elementTypeSet = false;
	bool resSet         = false;
	bool spacingSet     = false;
	bool filenameSet    = false;
	
	ifstream f;
	f.open( filename );
	if( !f.is_open() )
		return false;
	
	int lineNumber=0;
	string line;
	while( getline( f, line ) )
	{
		lineNumber++;
VOLUMEDATA_DEBUG_HEAD_PRINT("[" << lineNumber << "] " << line << endl)
		
		// skip empty lines
		if( line.empty() )
			continue;
		
		// tokenize
		vector<string> tokens;
		stringstream ss( line );
		string token;
VOLUMEDATA_DEBUG_HEAD_PRINT("Line " << lineNumber << " tokens: ")
		while( ss >> token )
		{
VOLUMEDATA_DEBUG_HEAD_PRINT("<" << token << ">")
			tokens.push_back( token );
		}
VOLUMEDATA_DEBUG_HEAD_PRINT(endl)
		
		// valid MHD lines have the format: "<Name> = <Var1> [<Var2> ...]"
		bool validLine = true;
		validLine &= ( tokens.size() >= 3 );     // at least 3 tokens
		if( tokens.size() >= 2 )
			validLine &= ( tokens[1] == "=" );  // 2nd token must be '='
		
		// skip invalid lines
		if( !validLine )
		{
			cerr << "Warning: Invalid MHD statement in line " << lineNumber << ":" << endl;
			cerr << "        " << line << endl;
			continue;
		}
		
		// parse MHD statement
		//_____________________________________________			
		if( tokens[0] == "NDims" )
		{
			if(!( tokens[2]=="3" || tokens[2]=="2" || tokens[2]=="1" ))
			{
				cerr << "Error: Only 1, 2 or 3 dimensional volume datasets are supported!" << endl;
				cerr << "       The given dataset states NDims = " << tokens[2] << endl;
				return false;
			}
		}
		else
		//_____________________________________________				
		if( tokens[0] == "DimSize" )
		{
			if( resSet )
			{
				cerr << "Warning: Duplicate statement 'DimSize' found!" << endl;
				continue;
			}			

			// Number of given dimension values
			int givenDimensions = tokens.size() - 2; // either 1, 2 or 3

			// Sanity check
			if( givenDimensions < 1 || givenDimensions > 3 )
			{
				cerr << "Error: Only 1, 2 or 3 dimensional volume datasets are supported!" << endl;
				cerr << "       The given dataset states DimSize = ";
				for( unsigned int i=2; i < tokens.size(); ++i )
					cerr << token[i] << " ";
				return false;
			}			
			
			// Set non given dimensions to 1
			resX = atoi( tokens[2].c_str() );
			if( givenDimensions > 1 )
				resY = atoi( tokens[3].c_str() );
			else
				resY = 1;
			if( givenDimensions > 2 )
				resZ = atoi( tokens[4].c_str() );
			else
				resZ = 1;
			
VOLUMEDATA_DEBUG_HEAD_PRINT("Parsed resolution " << resX << "," << resY << "," << resZ << endl)
			
			if( resX==0 || resY==0 || resZ==0
				|| resX<0 || resY<0 || resZ<0
#ifdef INT_MAX
				|| resX==INT_MAX || resY==INT_MAX || resZ==INT_MAX
				|| resX==INT_MIN || resY==INT_MIN || resZ==INT_MIN
#endif
			    )
			{
				cerr << "Error: Volume resolution does not compute!" << endl;
				cerr << "       The given dataset states DimSize = ";
				for( unsigned int i=2; i < tokens.size(); ++i )
					cerr << token[i] << " ";
				return false;
			}
			
			resSet = true;
		}
		else
		//_____________________________________________				
		if( tokens[0] == "ElementType" )
		{
			if( elementTypeSet )
			{
				cerr << "Warning: Duplicate statement 'ElementType' found!" << endl;
				continue;
			}
			
			// Local naming conventions follows MHD naming, we only have to 
			// append the "MET_" prefix.
			bool elementTypeFound = false;
			for( int i=0; i < NumberOfElementTypes; ++i )
			{
				string mhd_typestring = string("MET_") + string(ElementTypeString[i]);
				if( mhd_typestring.compare(tokens[2])==0 )
				{
					// Element type names are consecutively numbered from 0
					elementType = (ElementTypeName)i;
					elementTypeFound = true;
				}
			}

			if( !elementTypeFound )
			{
				cerr << "Error: Unsupported data type \""<< tokens[2] << "\"!" << endl;
				return false;
			}
			if( tokens.size() > 3 )
			{
				cerr << "Warning: Additional tokens on line " << lineNumber << ":" << endl;
				cerr << "        " << line << endl;
			}			
			
			elementTypeSet = true;
		}
		else
		//_____________________________________________				
		if( tokens[0] == "ElementSpacing" )
		{
			if( spacingSet )
			{
				cerr << "Warning: Duplicate statement 'ElementSpacing' found!" << endl;
				continue;
			}
			
			// Number of given dimension values
			int givenDimensions = tokens.size() - 2; // either 1, 2 or 3

			//if( tokens.size() != 2+3 )
			//{
			//	cerr << "Error: Only 3 dimensional volume datasets are supported!" << endl;
			//	cerr << "       The given dataset states ElementSpacing = ";
			//	for( unsigned int i=2; i < tokens.size(); ++i )
			//		cerr << token[i] << " ";
			//	cerr << endl;
			//	return false;
			//}
			
			spacingX = atof( tokens[2].c_str() );
			if( givenDimensions > 1 )
				spacingY = atof( tokens[3].c_str() );
			else
				spacingY = 1.0;
			if( givenDimensions > 2 )
				spacingZ = atof( tokens[4].c_str() );
			else
				spacingZ = 1.0;
			
VOLUMEDATA_DEBUG_HEAD_PRINT("Parsed spacing " << spacingX << "," << spacingY << "," << spacingZ << endl)
			
			if( spacingX==0 || spacingY==0 || spacingZ==0
				|| spacingX<0 || spacingY<0 || spacingZ<0
				|| fabs(spacingX)==HUGE_VAL || fabs(spacingY)==HUGE_VAL || fabs(spacingZ)==HUGE_VAL )
			{
				cerr << "Error: Volume slice spacing does not compute!" << endl;
				cerr << "       The given dataset states ElementSpacing = ";
				for( unsigned int i=2; i < tokens.size(); ++i )
					cerr << token[i] << " ";
				cerr << endl;
				return false;
			}
			
			spacingSet = true;
		}
		else
		//_____________________________________________
		if( tokens[0] == "ElementDataFile" )
		{
			if( filenameSet )
			{
				cerr << "Warning: Duplicate statement 'ElementDataFile' found!" << endl;
				continue;
			}
							
			objectFilename = tokens[2];					
			filenameSet = true;
		}
		else
		//_____________________________________________
		if( tokens[0] == "ElementNumberOfChannels" )		// OPTIONAL
		{
			numChannels = atoi( tokens[2].c_str() );
VOLUMEDATA_DEBUG_HEAD_PRINT("Parsed number of channels " << numChannels << endl)
			if( numChannels < 1 )
			{
				cerr << "Error: Volume number of channels per element < 1!" << endl;
				return false;
			}
		}
		else
		{
VOLUMEDATA_DEBUG_HEAD_PRINT("Ignoring line " << lineNumber << ":" << endl)
VOLUMEDATA_DEBUG_HEAD_PRINT(line << endl)
			continue;
		}
	}
	
	if( !filenameSet || !elementTypeSet || !resSet || !spacingSet )
	{
		cerr << "Error: MHD file must specify ElementDataFile, ElementType, DimSize and ElementSpacing!" << endl;
		return false;
	}
VOLUMEDATA_DEBUG_HEAD_PRINT("Succesfully parsed MHD file" << endl)
	
	// set VolumeDataHeader members
	m_elementType = elementType;
	m_numChannels = numChannels;
	m_resX = resX;
	m_resY = resY;
	m_resZ = resZ;
	m_spacingX = spacingX;
	m_spacingY = spacingY;
	m_spacingZ = spacingZ;
	m_filename = objectFilename;

	return true;
}


//-----------------------------------------------------------------------------
/**
  \class VolumeDataHeaderLoaderDAT

  For a description of the QVis / Voreen .DAT volume header fileformat see:
  http://openqvis.sourceforge.net/docu/fileformat.html

  This is nearly identical code as in VolumeDataHeaderLoaderMHD since both
  formats (as used w/o meta information as in our context) mainly differ in
  the used variable names and the separator.

  Example .DAT file:
  \verbatim
	 ObjectFileName: CT_Head_large.raw
	 TaggedFileName: ---
	 Resolution: 512 512 106
	 SliceThickness: 0.435547 0.435547 2.0
	 Format: UCHAR
	 NbrTags: 0
	 ObjectType: TEXTURE_VOLUME_OBJECT
	 ObjectModel: RGBA
	 GridType: EQUIDISTANT
  \endverbatim

  Like for QVren we assume ObjectType to be TEXTURE_VOLUME_OBJECT, ObjectModel 
  to be RGB and GridType to be set to EQUIDISTANT.

*/
//-----------------------------------------------------------------------------

bool VolumeDataHeaderLoaderDAT::load( const char* filename )
{
	int    resX, resY, resZ;
	double spacingX, spacingY, spacingZ;
	string objectFilename;
	VolumeDataHeader::ElementTypeName elementType;

	bool elementTypeSet = false;
	bool resSet         = false;
	bool spacingSet     = false;
	bool filenameSet    = false;
	
	ifstream f;
	f.open( filename );
	if( !f.is_open() )
		return false;
	
	int lineNumber=0;
	string line;
	while( getline( f, line ) )
	{
		lineNumber++;
VOLUMEDATA_DEBUG_HEAD_PRINT("[" << lineNumber << "] " << line)
		
		// tokenize
		vector<string> tokens;
		stringstream ss( line );
		string token;
VOLUMEDATA_DEBUG_HEAD_PRINT("Line " << lineNumber << " tokens: ")
		while( ss >> token )
		{
VOLUMEDATA_DEBUG_HEAD_PRINT("<" << token << ">")
			tokens.push_back( token );
		}
VOLUMEDATA_DEBUG_HEAD_PRINT(endl)
		
		// valid DAT lines have the format: "<Name>: <Var1> [<Var2> ...]"
		bool validLine = true;
		validLine &= ( tokens.size() >= 2 );     // at least 2 tokens (':' may be last char of first token)
		if( tokens.size() >= 2 )
		{
			if( !(tokens[1] == ":") )
			{
				// OK, last character of first token must be ':'
				if( *(tokens[0].end()-1) == ':' )
				{
					// strip "=" from first token
					tokens[0] = tokens[0].substr(0,tokens[0].length()-1);
					// insert extra ":" token before current second token
					tokens.insert( tokens.begin()+1, ":" );					
				}
				else
					validLine = false;
			}
		}
		
		// skip invalid lines
		if( !validLine )
		{
			cerr << "Warning: Invalid DAT statement in line " << lineNumber << ":" << endl;
			cerr << "        " << line << endl;
			continue;
		}
		
		// parse relevant DAT statement
		//_____________________________________________				
		if( tokens[0] == "Resolution" )  // MHD "DimSize"
		{
			if( resSet )
			{
				cerr << "Warning: Duplicate statement 'Resolution' found!" << endl;
				continue;
			}
			
			if( tokens.size() != 2+3 )
			{
				cerr << "Error: Only 3 dimensional volume datasets are supported!" << endl;
				cerr << "       The given dataset states DimSize = ";
				for( unsigned int i=2; i < tokens.size(); ++i )
					cerr << token[i] << " ";
				return false;
			}
			
			resX = atoi( tokens[2].c_str() );
			resY = atoi( tokens[3].c_str() );
			resZ = atoi( tokens[4].c_str() );
			
VOLUMEDATA_DEBUG_HEAD_PRINT("Parsed resolution " << resX << "," << resY << "," << resZ << endl)
			
			if( resX==0 || resY==0 || resZ==0
				|| resX<0 || resY<0 || resZ<0
#ifdef INT_MAX
				|| resX==INT_MAX || resY==INT_MAX || resZ==INT_MAX
				|| resX==INT_MIN || resY==INT_MIN || resZ==INT_MIN
#endif
			    )
			{
				cerr << "Error: Volume resolution does not compute!" << endl;
				cerr << "       The given dataset states DimSize = ";
				for( unsigned int i=2; i < tokens.size(); ++i )
					cerr << token[i] << " ";
				return false;
			}
			
			resSet = true;
		}
		else
		//_____________________________________________				
		if( tokens[0] == "Format" )			// MHD "ElementType"
		{
			if( elementTypeSet )
			{
				cerr << "Warning: Duplicate statement 'Format' found!" << endl;
				continue;
			}			
			if( (tokens[2] == "UCHAR") )	// MHD "MET_UCHAR"
			{
				elementType = VolumeDataHeader::UCHAR;
			}
			else
			if( (tokens[2] == "FLOAT") )	// MHD "MET_FLOAT"
			{
				elementType = VolumeDataHeader::FLOAT;
			}
			else
			{
				cerr << "Error: Currently only UCHAR and FLOAT datasets for DAT supported!" << endl;
				cerr << "       The given dataset states Format = " << tokens[2];
				return false;
			}
			if( tokens.size() > 3 )
			{
				cerr << "Warning: Additional tokens on line " << lineNumber << ":" << endl;
				cerr << "        " << line << endl;
			}			
			
			elementTypeSet = true;
		}
		else
		//_____________________________________________				
		if( tokens[0] == "SliceThickness" )	// MHD "ElementSpacing"
		{
			if( spacingSet )
			{
				cerr << "Warning: Duplicate statement 'SliceThickness' found!" << endl;
				continue;
			}
			
			if( tokens.size() != 2+3 )
			{
				cerr << "Error: Only 3 dimensional volume datasets are supported!" << endl;
				cerr << "       The given dataset states ElementSpacing = ";
				for( unsigned int i=2; i < tokens.size(); ++i )
					cerr << token[i] << " ";
				return false;
			}
			
			spacingX = atof( tokens[2].c_str() );
			spacingY = atof( tokens[3].c_str() );
			spacingZ = atof( tokens[4].c_str() );
			
VOLUMEDATA_DEBUG_HEAD_PRINT("Parsed spacing " << spacingX << "," << spacingY << "," << spacingZ << endl)
			
			if( spacingX==0 || spacingY==0 || spacingZ==0
				|| spacingX<0 || spacingY<0 || spacingZ<0
				|| fabs(spacingX)==HUGE_VAL || fabs(spacingY)==HUGE_VAL || fabs(spacingZ)==HUGE_VAL )
			{
				cerr << "Error: Volume slice spacing does not compute!" << endl;
				cerr << "       The given dataset states SliceThickness = ";
				for( unsigned int i=2; i < tokens.size(); ++i )
					cerr << token[i] << " ";
				return false;
			}
			
			spacingSet = true;
		}
		else
		//_____________________________________________
		if( tokens[0] == "ObjectFileName" )		// MHD "ElementDataFile"
		{
			if( filenameSet )
			{
				cerr << "Warning: Duplicate statement 'ObjectFileName' found!" << endl;
				continue;
			}

			objectFilename = tokens[2];					
			filenameSet = true;
VOLUMEDATA_DEBUG_HEAD_PRINT("Parsed object filename '" << objectFilename << "'" << endl)
		}
		else
		{
VOLUMEDATA_DEBUG_HEAD_PRINT("Ignoring line " << lineNumber << ":" << endl)
VOLUMEDATA_DEBUG_HEAD_PRINT(line << endl)
			continue;
		}
	}
	
	if( !filenameSet || !elementTypeSet || !resSet || !spacingSet )
	{
		cerr << "Error: MHD file must specify ElementDataFile, ElementType, DimSize and ElementSpacing!" << endl;
		return false;
	}
VOLUMEDATA_DEBUG_HEAD_PRINT("Succesfully parsed DAT file" << endl)
	
	// set VolumeDataHeader members
	m_elementType = elementType;
	m_resX = resX;
	m_resY = resY;
	m_resZ = resZ;
	m_spacingX = spacingX;
	m_spacingY = spacingY;
	m_spacingZ = spacingZ;
	m_filename = objectFilename;

	return true;
}



//-----------------------------------------------------------------------------
//  VolumeDataBase
//-----------------------------------------------------------------------------

#define VOLUMEDATA_LOAD_MACRO(TYPE) \
	{ \
		VolumeDataLoaderRAW<TYPE>  loader;          \
		VolumeData         <TYPE>* data(NULL);      \
		data = loader.load( filename, mhd );        \
		if( data ) {                                \
			return (VolumeDataBase*) data;          \
		} \
	}

VolumeDataBase* VolumeDataBase::load_raw( const char* filename, VolumeDataHeader* mhd )
{	
	// try to load type given in header
	if( mhd->elementTypeName() == UCHAR ) VOLUMEDATA_LOAD_MACRO(unsigned char)  else
	if( mhd->elementTypeName() == CHAR  ) VOLUMEDATA_LOAD_MACRO(char)           else
	if( mhd->elementTypeName() == USHORT) VOLUMEDATA_LOAD_MACRO(unsigned short) else
	if( mhd->elementTypeName() == SHORT ) VOLUMEDATA_LOAD_MACRO(short)          else
	if( mhd->elementTypeName() == FLOAT ) VOLUMEDATA_LOAD_MACRO(float)          else
	if( mhd->elementTypeName() == DOUBLE) VOLUMEDATA_LOAD_MACRO(double)         else
	{
		// unsupported type
		cerr << "Error: Unsupported volume type!\n";
	}		

	//// try all supported types
	//VOLUMEDATA_LOAD_MACRO(unsigned char)
	//VOLUMEDATA_LOAD_MACRO(float)
	//VOLUMEDATA_LOAD_MACRO(unsigned short)
	//VOLUMEDATA_LOAD_MACRO(char)
	//VOLUMEDATA_LOAD_MACRO(short)
	//VOLUMEDATA_LOAD_MACRO(double)

	return NULL;
}
