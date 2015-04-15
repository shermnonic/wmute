#include "VideoPlayer.h"
#include "glbase.h"
#include <glutils/GLError.h>
#ifdef GL_NAMESPACE
using GL::checkGLError;
#endif
#include <iostream>
using std::cerr;
using std::endl;

//-----------------------------------------------------------------------------
//  Global callbacks
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#if 1
    // Boost mutex
    #include <boost/thread/mutex.hpp>
    boost::mutex videoMutex;
    #define VIDEO_LOCK videoMutex.lock();
    #define VIDEO_UNLOCK videoMutex.unlock();
#else
    // C++11 mutex
    #include <mutex>
    std::mutex videoMutex;
    #define VIDEO_LOCK videoMutex.lock();
    #define VIDEO_UNLOCK videoMutex.unlock();
#endif

// Global variables
uint8_t     *videoBuffer     = 0;
unsigned int videoBufferSize = 0;
uint8_t     *pixelBuffer = 0;  // FIXME: Free pixelbuffer!
int videoWidth  = 0;
int videoHeight = 0;

void cbVideoPrerender(void *p_video_data, uint8_t **pp_pixel_buffer, int size) 
{
    // Locking
    VIDEO_LOCK

    //printf("cbVideoPrerender %
    //printf("vtest: %lld\n",(long long int)p_video_data);

    if( (unsigned)size > videoBufferSize || !videoBuffer )
    {
        printf("Reallocate raw video buffer\n");
        free(videoBuffer);
        videoBuffer = (uint8_t *)malloc(size);
        videoBufferSize = size;
    }
    *pp_pixel_buffer = videoBuffer;
}

void cbVideoPostrender(void *p_video_data, uint8_t *p_pixel_buffer
      , int width, int height, int pixel_pitch, int size, int64_t pts)
{
    //printf("cbVideoPostrender %i\n",size);

    // See also
    // http://stackoverflow.com/questions/23092940/get-frame-from-video-with-libvlc-smem-and-convert-it-to-opencv-mat-c

    videoWidth = width;
    videoHeight = height;

    if( width!=videoWidth || height!=videoHeight || !pixelBuffer )
    {
        free( pixelBuffer );
        pixelBuffer = (uint8_t *)malloc( size );
    }

    memcpy( pixelBuffer, p_pixel_buffer, size );

    // Unlocking
    VIDEO_UNLOCK
}

//-----------------------------------------------------------------------------
//  VideoPlayer implementation
//-----------------------------------------------------------------------------

VideoPlayer::VideoPlayer()
: m_vlcInstance( NULL ),
  m_vlcPlayer( NULL ),
  m_width(0),
  m_height(0),
  m_sizeChanged(true),
  m_texid(0)
{
	init();
}

VideoPlayer::~VideoPlayer()
{
	destroy();
}

bool VideoPlayer::init()
{
	// VLC options
	char smem_options[1000];
	sprintf(smem_options
      , "#transcode{vcodec=RV24}:smem{"
		 "video-prerender-callback=%lld,"
		 "video-postrender-callback=%lld,"
		 "video-data=%lld},"
	  , (long long int)(intptr_t)(void*)&cbVideoPrerender
	  , (long long int)(intptr_t)(void*)&cbVideoPostrender
	  , (long long int)200); // Test data

//	  , "#transcode{vcodec=I444,acodec=s16l}:smem{"
// Audio data is omitted:
//		 "audio-prerender-callback=%lld,"
//		 "audio-postrender-callback=%lld,"
//		 "audio-data=%lld,"
//	  , (long long int)(intptr_t)(void*)&cbAudioPrerender
//	  , (long long int)(intptr_t)(void*)&cbAudioPostrender
//	  , (long long int)100   // This would normally be useful data, 100 is just test data

// Also have a look at:
//      libvlc_video_set_callbacks( ... )
//      libvlc_video_set_format( mp, "RV24", W,H, W * 3 );

	const char * const vlc_args[] = {
		  "-I", "dummy", // Don't use any interface
		  "--ignore-config", // Don't use VLC's config
		  "--extraintf=logger", // Log anything
		  "--verbose=1", // Be verbose
		  "--sout", smem_options // Stream to memory
		   };
	
    /* Initialize libVLC */
    m_vlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);

    /* Complain in case of broken installation */
    if( m_vlcInstance == NULL ) 
	{
        cerr << "VideoPlayer: Could not init libVLC" << endl;
        return false;
    }
	
	return true;
}

void VideoPlayer::destroy()
{
	if( m_vlcInstance )
	{
		stop();
		libvlc_release( m_vlcInstance );
	}
}

bool VideoPlayer::openFile( const char* filename )
{
	if( !m_vlcInstance )
		return false;
	
	// Stop if something is playing
    if( m_vlcPlayer && libvlc_media_player_is_playing( m_vlcPlayer ) )
		stop();
	
	/* Create new media */
    libvlc_media_t *vlcMedia = libvlc_media_new_path( m_vlcInstance, filename );
    if (!vlcMedia)
        return false;

    /* Create a new libvlc player */
    m_vlcPlayer = libvlc_media_player_new_from_media( vlcMedia );

    /* Release the media */
    libvlc_media_release( vlcMedia );
	
	// NOTE: Here we would usually integrate the video into the interface
	//       but that is omitted here since we just want to extract the frame
	//       data.
	
	/* And start playback */
    libvlc_media_player_play( m_vlcPlayer );
	
	return true;
}

void VideoPlayer::play()
{
	if( !m_vlcPlayer )
		return;
	
	if( libvlc_media_player_is_playing( m_vlcPlayer ) )
	{
		// Pause
		libvlc_media_player_pause( m_vlcPlayer );
	}
	else
	{
		// Play again
		libvlc_media_player_play( m_vlcPlayer );
	}
}

void VideoPlayer::stop()
{
	if( !m_vlcPlayer )
		return;	
	
	// Stop media player
	libvlc_media_player_stop( m_vlcPlayer );
	
	// Release
	libvlc_media_player_release( m_vlcPlayer );
	
	m_vlcPlayer = NULL;
}

void VideoPlayer::poll()
{
    VIDEO_LOCK

    if( videoBuffer && videoBufferSize > 0
        && videoWidth > 0 && videoHeight > 0 && pixelBuffer)
    {
        m_sizeChanged = ( m_width != videoWidth || m_height != videoHeight );
        m_width  = videoWidth;
        m_height = videoHeight;

        updateTexture();
    }
    else
    {
        cerr << "VideoPlayer : Invalid video data while polling!" << endl;
    }

    VIDEO_UNLOCK
}

void VideoPlayer::updateTexture()
{
    if( m_texid <= 0 )
        return;

    glBindTexture( GL_TEXTURE_2D, m_texid );
    if( hasSizeChanged() )
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_width,m_height, 0,
                      GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pixelBuffer );
    else
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0,0, m_width,m_height,
                      GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)pixelBuffer );

    checkGLError("VideoPlayer::updateTexture()");
}


