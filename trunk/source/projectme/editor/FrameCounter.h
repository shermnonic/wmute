#ifndef FRAMECOUNTER_H
#define FRAMECOUNTER_H

/// Stupid and imprecise FPS counter (based on clock() function)
class FrameCounter
{
public:
	FrameCounter()
	: m_initialized(false), m_numFrames(0), m_fps(0.f)
	{}
		
	float measure()
	{
		if( !m_initialized )
		{
			// Start integration, return 0 FPS
			m_t0 = clock();
			m_fps = 0.f;
			m_numFrames = 0;
			m_initialized = true;
		}
		else
		{
			// Count frames
			m_numFrames++;
			
			// Check if integration period exceeded
			std::clock_t now = clock();
			float dt = (float)(now - m_t0) / CLOCKS_PER_SEC;
			if( dt > 1.5 ) // Integrate over 1.5 seconds
			{
				// Period exceeded, update FPS estimate
				m_fps = (float)m_numFrames / dt;
				m_t0 = now;
				m_numFrames = 0;
			}
		}		
		return m_fps;		
	}
	
private:
	bool m_initialized;
	unsigned m_numFrames;
	std::clock_t m_t0; // Last timepoint of measurement
	float m_fps; // FPS estimate from last integrated measurement
};

#endif // FRAMECOUNTER_H
