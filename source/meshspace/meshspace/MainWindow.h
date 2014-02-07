// MainWindow for meshspace application
// Max Hermann, Jan 2014
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class SceneViewer;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

public slots:
	void openMesh();
	void openAnimation();
	void saveMesh();

protected:
	void closeEvent( QCloseEvent* event );

	///@{ Application settings
	void readSettings();
    void writeSettings();
	QString m_baseDir; ///< directory of last successfully opened file
	///@}
	
private:
	SceneViewer* m_viewer;
};

#endif // MAINWINDOW
