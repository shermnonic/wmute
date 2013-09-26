Overview e8 - Simplistic demo engine
====================================
Max Hermann, September 2013

- base
	- AbstractRenderer
	- AbstractInteractor

	- Module (depends on lib params)

	- ParameterBase (lib params)
		- ...
		
	(Plain OpenGL functionality classes)
	- TrackballInteractor

- viewer (glfw based)

- editor (Qt based)
	- EditorWidget
		* void setRenderer( AbstractRenderer* scene );
		* void setInteractor( AbstractInteractor* interactor );

	- PropertyTreeWidget (lib qparams)