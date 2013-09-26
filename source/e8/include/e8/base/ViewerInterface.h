#ifndef VIEWERINTERFACE_H
#define VIEWERINTERFACE_H

//#include <e8/base/AbstractRenderer.h>
//#include <e8/base/AbstractInteractor.h>

class AbstractRenderer;
class AbstractInteractor;

/// Virtual base class defining the interface for all e8 viewers.
class ViewerInterface
{
	virtual void setRenderer( AbstractRenderer* scene )=0;
	virtual void setInteractor( AbstractInteractor* interactor )=0;
};

#endif // VIEWERINTERFACE_H
