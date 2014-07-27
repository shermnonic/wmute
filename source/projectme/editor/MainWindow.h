// MainWindow for projectme editor
// Max Hermann, Jul 2014
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "RenderSet.h"

class QMdiArea;
class QGLWidget;
class QMenu;

class RenderSetWidget;

/** @addtogroup editor_grp Editor
  * @{ */

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

public slots:
	void open();
	void save();

private slots:
	void newPreview();
	void newScreen();
	void updateViewMenu();

protected:
	void closeEvent( QCloseEvent* event );

	///@{ Application settings
	void readSettings();
    void writeSettings();
	QString m_baseDir; ///< directory of last successfully opened file
	///@}
	
private:
	QMdiArea * m_mdiArea;
	QGLWidget* m_sharedGLWidget;

	RenderSetManager m_renderSetManager;
	
	QMenu* m_menuView;
	QList<RenderSetWidget*> m_previews;
	QList<RenderSetWidget*> m_screens;
};

/** @} */ // end group

#endif // MAINWINDOW
