#ifndef MONROPRESSINGPHASESPACE_H
#define MONROPRESSINGPHASESPACE_H

#include <vector>

/// Phase space embedding of sound samples as proposed in [Monro&Pressing1998]
class MonroPressingPhaseSpace
{
public:
	MonroPressingPhaseSpace()
	: m_phaseShift(42), 
	  m_pointBufferOfs(0),
	  m_scale(10.f)
	{}
	
	/// Supply n new wave samples to embed into phase space
	void pollWaveSamples( short* samples, int n );

	float getScale() const { return m_scale; }
	void setScale( float s ) { m_scale = s; }

	int getPhaseShift() const { return m_phaseShift; }
	void setPhaseShift( int p ) { m_phaseShift = (p>0)?p:1; }

	int getNumPoints() const { return (int)m_pointBuffer.size(); }

	float* getPointBuffer() { return &m_pointBuffer[0]; }
	int getStartPoint() const { return m_pointBufferOfs/3; }
	int getLenFront() const { return (int)(m_pointBuffer.size() - m_pointBufferOfs)/3-1; }
	int getLenBack() const { return m_pointBufferOfs/3; }
	
private:
	int m_phaseShift; // phase shift, same for all dimensions
	std::vector<float> m_pointBuffer;
	int m_pointBufferOfs; // position in buffer separating old and new samples
	float m_scale;
};

#endif // MONROPRESSINGPHASESPACE_H
