#ifndef RGBDVIEWERMAINWINDOW_H
#define RGBDVIEWERMAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "RGBDMovie.h"

const QString APP_NAME        ( "rgbdviewer" );
const QString APP_ORGANIZATION( "www.386dx25.de" );
#define       APP_ICON        QIcon(QPixmap(":/data/icons/icon.png"))

class RendererWidget;
class QTimer;
class QTime;

class RGBDViewerMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	RGBDViewerMainWindow();

public slots:
	void openDataset();
	void saveDataset();	

	void togglePlayPause();
	void playMovie( bool toggle );

protected slots:
	void updateAnimation();

protected:
	void closeEvent( QCloseEvent* event );

private:
	// Application settings
	void readSettings();
    void writeSettings();
	QString m_baseDir;

	RendererWidget* m_rendererWidget;
	RGBDMovie       m_movie;
	
	QTimer*         m_animationTimer;
	QTime*          m_playingTime;

	QAction*        m_actPlayPause;
};

#endif // RGBDVIEWERMAINWINDOW_H
