#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QString>
#include <QDebug>
#include "MainWindow.h"

int main( int argc, char* argv[] )
{
	Q_INIT_RESOURCE( projectme );

	QApplication app( argc, argv );

#if 0
	QFile f(":/style.css");	
	if( f.open(QIODevice::ReadOnly | QIODevice::Text) )
	{		
		qDebug("Setting style sheet");
		QString style = QTextStream(&f).readAll();
		qDebug(style.toStdString().c_str());
		app.setStyleSheet( style );
	}
#endif

	MainWindow mw;
	mw.show();

	return app.exec();;
}
