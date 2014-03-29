#ifndef TENSORFIELDOBJECTWIDGET_H
#define TENSORFIELDOBJECTWIDGET_H

#include <QWidget>

// Forward declarations
namespace scene
{
	class TensorfieldObject;
}
class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;
class QPushButton;

/** @addtogroup meshspaceGUI_grp meshspace GUI
  * @{ */

/// GUI for TensorfieldObject
class TensorfieldObjectWidget : public QWidget
{
	Q_OBJECT

signals:
	void redrawRequired();
	
public:
	TensorfieldObjectWidget( QWidget* parent=0 );

	void setMaster( scene::TensorfieldObject* master );

protected slots:
	void updateMaster();

	void loadTensors();
	void saveTensors();

private:
	scene::TensorfieldObject* m_master;

	QDoubleSpinBox* m_dsbGlyphScale;
	QDoubleSpinBox* m_dsbGlyphSharpness;
	QSpinBox*       m_spbGlyphResolution;
	QCheckBox*      m_chkGlyphSqrtEV;
	QPushButton*    m_butLoadTensors;
	QPushButton*    m_butSaveTensors;
};

/** @} */ // end group

#endif // TENSORFIELDOBJECTWIDGET_H
