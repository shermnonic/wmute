// ObjectPropertiesWidget - scene::Object properties for SceneViewer
// Max Hermann, Jan 2014
#ifndef OBJECTPROPERTIESWIDGET_H
#define OBJECTPROPERTIESWIDGET_H

#include <QWidget>
#include <QPixmap>
#include "scene.h"

class QLabel;
class QPushButton;
class QSlider;
class QLineEdit;
class QTextEdit;
class QScrollArea;

class TensorfieldObjectWidget;
class MultiSliderWidget;

/** @addtogroup meshspaceGUI_grp meshspace GUI
  * @{ */

/**
	Inspector to show and edit scene::Object properties for \a SceneViewer.
	Specialized functionality is provided for the following scene objects:
	- \a MeshObject
	- \a PCAObject
	- \a TensorfieldObject
*/
class ObjectPropertiesWidget : public QWidget
{
	Q_OBJECT

signals:
	/// The signal is emitted when a redraw is required.
	void redrawRequired();
	void modelChanged();

public:
	ObjectPropertiesWidget( QWidget* parent=0 );
	
public slots:
	void setSceneObject( scene::Object* obj );
	void reset();

protected slots:
	// set.. functions set the ui state (w/o changing the scene object)
	void setColor( scene::Color color );
	// change.. functions manipulate the selected scene object directly
	void changeName( QString name );
	void changeColor();
	void changeFrame( int frame );
	void changePCACoeff( int mode, int ivalue );
	
private:
	QLineEdit* m_leName;
	QPixmap    m_pixmapColor;
	QPushButton* m_butColor;
	QSlider*   m_sliderFrame;
	QTextEdit* m_teInfo;

	TensorfieldObjectWidget* m_tensorfieldObjectWidget;
	MultiSliderWidget* m_pcaSlider;
	QScrollArea* m_pcaSliderArea;

	scene::Object* m_obj;
};

/** @} */ // end group

#endif // OBJECTPROPERTIESWIDGET_H
