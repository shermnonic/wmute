Simple 3D viewer for RGBD movies
--------------------------------
Max Hermann, Jan 2013

For Visual Studio execute "copydata.bat" first to copy the required data files locally to this source code directory.


Some TODO's:

- "Live" preview via OpenNI
	- realtime input
	- replay of OpenNI raw format

- VBO object should be shared across frames (currently each frame allocates
  in addition to its data buffer also own VBO buffers)

- Performant binary format
  - zipped / run-length-encoded float raw streams
  - 2x ffmpeg streams?

- Non-float parameters

- Non-local filters

- Video-playback in background
