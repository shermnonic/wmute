#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <vlc/vlc.h>

/**
    Play video and extract frame data via libVLC/smem, no audio so far.
    Should be tboost mutex examplereated as singleton!
*/
class VideoPlayer
{
public:
	VideoPlayer();
	~VideoPlayer();

	bool openFile( const char* filename );
    void setTexture( int texid ) { m_texid = texid; }

    //@{ Playback controls
    void play();
	void stop();
    //@}

    //@{ Polling mechanism
    void poll();
    bool hasSizeChanged() const { return m_sizeChanged; }
    int  width() const { return m_width; }
    int  height() const { return m_height; }
    //@}

protected:
	bool init();
	void destroy();
    void updateTexture();

private:
	libvlc_instance_t*     m_vlcInstance;
	libvlc_media_player_t* m_vlcPlayer;

    int m_width, m_height;
    bool m_sizeChanged;

    int m_texid;
};

#endif // VIDEOPLAYER_H
