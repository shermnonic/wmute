#include "MonroPressingPhaseSpace.h"
#include <iostream>

void MonroPressingPhaseSpace::pollWaveSamples( short* samples, int n )
{	
	if( n<=0 )
		return;

	int numUsableSamples = n - (DIM-1)*m_phaseShift;
	
	// Point buffer should be at least twice as large as sample buffer
	if( 2*numUsableSamples > m_pointBuffer.size()/DIM )
	{
		m_pointBuffer.resize( 2*numUsableSamples*DIM );
		std::cout << "Enlarged phase space buffer to " << m_pointBuffer.size() << "\n";
	}

	// Normalize buffer
	short maxval = std::numeric_limits<short>::max();
	if( m_normalize )
	{
		maxval = 0;
		for( int i=0; i < n; i++ )
			maxval = std::max( samples[i], maxval );
	}	
	
	m_relmaxval = log( 1.f + fabs((float)maxval / (.5f*65536.f)) );

	int N   = (int)m_pointBuffer.size();
	int ofs = m_pointBufferOfs;
	for( int i=0; i < numUsableSamples; i++, ofs+=DIM )
	{	
		// insert new point into phase space
		for( int d=0; d < DIM; d++ )
		{
			// normalized sample in [-1,1]
			float v = (float)samples[i+ d * m_phaseShift] / (float)maxval; // was: /	(.5f*65536.f);

			// decibels 
			float sign = (v > 0.0) ? 1.0 : -1.0;
			v = log(1+fabs(v));
	
			// insert into buffer, wrapping around at beginning if required
			m_pointBuffer[ofs%N+d] = m_scale * sign * v;

			// float x=v[0], y=v[1];
			//v[0] = log(x+0.5); //1.f/x*x*x;
			//v[1] = log(y+0.5); //1.f/y*y*y;
		}
	}
	m_pointBufferOfs = ofs%N;

	// re-compute tangents
	updateTangents();
}

float* MonroPressingPhaseSpace::getTangentBuffer()
{
	if( m_tangentBuffer.size() != m_pointBuffer.size() )
		updateTangents();
	return &m_tangentBuffer[0];
}

void MonroPressingPhaseSpace::updateTangents()
{
	// number of points
	int n = m_pointBuffer.size()/3;
	m_tangentBuffer.resize( 3*n );
	
	for( int i=0; i < n-1; ++i )
	{
		// tangent(i) = point(i+1) - point(i)
		for( int d=0; d < DIM; ++d )
			m_tangentBuffer[i*DIM+d] = 
				m_pointBuffer[(i+1)*DIM+d] - m_pointBuffer[i*DIM+d];
	}

	// special treatment of last point
	// simply assign same tangent as previous one
	for( int d=0; d < DIM; ++d )
		m_tangentBuffer[(n-1)*DIM+d] = m_tangentBuffer[(n-2)*DIM+d];
}
