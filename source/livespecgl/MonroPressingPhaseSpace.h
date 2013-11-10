#ifndef MONROPRESSINGPHASESPACE_H
#define MONROPRESSINGPHASESPACE_H

#include <vector>

/// Phase space embedding of sound samples as proposed in [Monro&Pressing1998]
class MonroPressingPhaseSpace
{
	enum { DIM = 3 };
public:
	MonroPressingPhaseSpace()
	: m_phaseShift(42), 
	  m_pointBufferOfs(0),
	  m_scale(10.f),
	  m_normalize(false)
	{}
	
	/// Supply n new wave samples to embed into phase space
	void pollWaveSamples( short* samples, int n );

	float getScale() const { return m_scale; }
	void setScale( float s ) { m_scale = s; }

	int getPhaseShift() const { return m_phaseShift; }
	void setPhaseShift( int p ) { m_phaseShift = (p>0)?p:1; }

	int getNumPoints() const { return (int)m_pointBuffer.size()/DIM; }

	float* getPointBuffer() { return &m_pointBuffer[0]; }
	int getStartPoint() const { return m_pointBufferOfs / DIM; }
	int getLenFront() const { return (int)(m_pointBuffer.size() - m_pointBufferOfs)/DIM-1; }
	int getLenBack() const { return m_pointBufferOfs / DIM; }

	float* getTangentBuffer();

	void setNormalize( bool b ) { m_normalize = b; }
	bool getNormalize() const { return m_normalize; }

	float getMaximum() const { return m_relmaxval; }

protected:
	void updateTangents();
	
private:
	int m_phaseShift; // phase shift, same for all dimensions
	std::vector<float> m_pointBuffer;  // embedded 3D vertices in linear order
	std::vector<float> m_tangentBuffer;// tangent at each point
	int m_pointBufferOfs; // position in buffer separating old and new samples
	float m_scale;
	bool m_normalize;
	float m_relmaxval; // relative maximum value in [0,1] (in last buffer)
};

#endif // MONROPRESSINGPHASESPACE_H
