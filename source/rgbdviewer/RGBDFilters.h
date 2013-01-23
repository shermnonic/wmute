#ifndef RGBDFILTERS_H
#define RGBDFILTERS_H

#include <limits>
#include <vector>
#include <string>

/// Generic filter operating on a single RGBD datum
class RGBDLocalFilter
{
public:
	virtual void processColor( float& r, float& g, float& b, float& a ) {}
	virtual void processPosition( float& x, float &y, float &depth, bool& discard ) {}

	/// Real valued parameter with name, description, min, max and default value
	struct FloatParameter
	{
		/// Default C'tor creates invalid parameter
		FloatParameter()
		    : invalid(true)
			{}
		/// Explicit C'tor
		FloatParameter( std::string name_, std::string  desc_, 
		                float value_,
		                float defaultValue_, float min_, float max_ )
			: invalid(false),
		      value(value_),
			  defaultValue(defaultValue_),
			  min(min_),
			  max(max_),
			  name(name_),
			  desc(desc_)
			{}
		bool invalid;
		float value, defaultValue, min, max;
		std::string name, desc;
	};

	typedef std::vector<FloatParameter*> FloatParameterVector;

	/// Returns parameter list
	FloatParameterVector getFloatParameters()
	{
		return m_floatParams;
	}

	/// Get parameter by name, returns NULL if no parameter of given name found
	FloatParameter* getFloatParameter( std::string name )
	{
		for( int i=0; i < m_floatParams.size(); i++ )
		{
			FloatParameter* p = m_floatParams[i];
			if( p && p->name == name )
				return p;
		}
		return NULL;
	}

	/// Get copy of parameter by name (changes will have no effect)
	FloatParameter getFloatParameter( std::string name, bool& found ) const
	{		
		for( int i=0; i < m_floatParams.size(); i++ )
		{
			FloatParameter* p = m_floatParams[i];
			if( p && p->name == name )
			{
				found = true;
				return *p;
			}
		}
		found = false;
		return FloatParameter(); // return empty (=invalid) parameter
	}

	/// Overloaded fnuction provided for convenience
	FloatParameter getFloatParameter( std::string name ) const
	{
		bool found;
		return getFloatParameter( name, found );
	}

protected:
	void registerParameter( FloatParameter* param )
	{
		m_floatParams.push_back( param );
	}

private:
	FloatParameterVector m_floatParams;
};

/// Meta Filter abstracting a whole queue of filters
/// Position and Color are treated separately.
class RGBDFilterQueue : public std::vector<RGBDLocalFilter*>, 
	                    public RGBDLocalFilter
{
public:
	void processColor( float& r, float& g, float& b, float& a ) 
	{
		for( int i=0; i < this->size(); i++ )
		{
			RGBDLocalFilter* filter = this->at(i);
			if( filter )
				filter->processColor( r, g, b, a );
		}
	}
	
	void processPosition( float& x, float &y, float &depth, bool& discard ) 
	{
		for( int i=0; i < this->size(); i++ )
		{
			RGBDLocalFilter* filter = this->at(i);
			if( filter )
				filter->processPosition( x, y, depth, discard );
		}
	}
};

/// Filter for scaling depth component
class RGBDDepthScale : public RGBDLocalFilter
{
public:
	RGBDDepthScale( float s=1.f )
	: m_paramDepthScale("Depth Scaling", "", s, 1.f, 0.f, 10.f )
	{
		registerParameter( &m_paramDepthScale );
	}

	void processPosition( float& x, float &y, float &depth, bool& discard ) 
	{
		depth *= m_paramDepthScale.value;
	}

private:
	FloatParameter m_paramDepthScale;
};

/// Filter for threshold depth to a [near,far] range
class RGBDDepthThreshold : public RGBDLocalFilter
{
public:
	enum { MAXDEPTH = 10000 };
	RGBDDepthThreshold( float near=0.f, float far=(float)MAXDEPTH )
	: m_paramNear( "Depth Threshold Near", "", near, 0.f, 0.f, (float)MAXDEPTH ),
	  m_paramFar ( "Depth Threshold Far" , "", far, (float)MAXDEPTH, 0.f, (float)MAXDEPTH  )
	{
		registerParameter( &m_paramNear );
		registerParameter( &m_paramFar );
	}

	void processPosition( float& x, float &y, float &depth, bool& discard ) 
	{
		float near = m_paramNear.value,
			  far  = m_paramFar.value;
		discard = (depth < near) || (depth > far);
	}

private:
	FloatParameter m_paramNear,
		           m_paramFar;
};

#endif // RGBDFILTERS_H
