#ifndef SOUNDINPUTWIDGET_H
#define SOUNDINPUTWIDGET_H

#include <QWidget>
class SoundInput;

class SoundInputWidget : public QWidget
{
	Q_OBJECT

public:
	SoundInputWidget( QWidget* parent=0 );

public slots:
	void setSoundInput( SoundInput* master );

protected slots:
	void openSoundFile();
	void openSoundInput();

	void restart();
	void play( bool toggle );

private:
	SoundInput* m_master;
};

#endif // SOUNDINPUTWIDGET_H
