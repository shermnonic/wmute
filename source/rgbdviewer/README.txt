Simple 3D viewer for RGBD movies
--------------------------------
Max Hermann, Jan 2013

For Visual Studio execute "copydata.bat" first to copy the required data files locally to this source code directory.


Some TODO's:

- Performant binary format
  - zipped / run-length-encoded float raw streams
  - 2x ffmpeg streams?

- Filters (local operations):
  - Depth threshold (min,max)
  - Depth scaling

- Video-playback in background
