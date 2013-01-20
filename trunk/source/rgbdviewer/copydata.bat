@rem Visual Studio and Qt4 resource files requires the data files in the source 
@rem code directory. Use this script to automatically copy the used icon images
@rem and possibly furter data files from trunk/data.
@rem Max Hermann, Jan 2013
mkdir data
cd data
mkdir icons
cd ..
copy ..\..\data\icons\icon.png               data\icons
copy ..\..\data\icons\icon.ico               data\icons
copy ..\..\data\icons\document-new.png       data\icons
copy ..\..\data\icons\document-open.png      data\icons
copy ..\..\data\icons\document-save.png      data\icons
copy ..\..\data\icons\media-playback-start.png   data\icons
copy ..\..\data\icons\media-playback-pause.png   data\icons
copy ..\..\data\icons\system-log-out.png     data\icons
pause
