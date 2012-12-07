#include "MonroPressingPhaseSpace.h"
#include <iostream>

void MonroPressingPhaseSpace::pollWaveSamples( short* samples, int n )
{	
	const int DIM = 3;
	int numUsableSamples = n - (DIM-1)*m_phaseShift;
	
	// Point buffer should be at least twice as large as sample buffer
	if( 2*numUsableSamples > m_pointBuffer.size()/DIM )
	{
		m_pointBuffer.resize( 2*numUsableSamples*DIM );
		std::cout << "Enlarged phase space buffer to " << m_pointBuffer.size() << "\n";
	}
	
	int N   = (int)m_pointBuffer.size();
	int ofs = m_pointBufferOfs;
	for( int i=0; i < numUsableSamples; i++, ofs+=DIM )
	{	
		// insert new point into phase space
		for( int d=0; d < DIM; d++ )
		{
			// normalized sample in [-1,1]
			float v = (float)samples[i+ d * m_phaseShift] / (.5f*65536.f);
			
			// insert into buffer, wrapping around at beginning if required
			m_pointBuffer[ofs%N+d] = m_scale * v;

			// float x=v[0], y=v[1];
			//v[0] = log(x+0.5); //1.f/x*x*x;
			//v[1] = log(y+0.5); //1.f/y*y*y;
		}
	}
	m_pointBufferOfs = ofs%N;
}
