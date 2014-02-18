#ifndef MULTISLIDERWIDGET_H
#define MULTISLIDERWIDGET_H

#include <QWidget>
#include <QVector>

class QSignalMapper;
class QSlider;
class QVBoxLayout;

class MultiSliderWidget : public QWidget
{
	Q_OBJECT
	
signals:
	void valueChanged( int slider_idx, int value );
	
public:
	MultiSliderWidget( QWidget* parent=0 );

	void createSliders( int num );

	void setValue( int slider_idx, int value );
	void setValues( QVector<int> values );
	void setValues( int same_value_for_all );

	void setRange( int slider_idx, int min_range, int max_range );
	void setRange( int min_range, int max_range );

	void getValuesAsDouble( std::vector<double>& values, double shift, double scale ) const;

protected slots:
	void onValueChange( int slider_idx );	
	
private:
	QSignalMapper* m_sigmap;
	QVector<QSlider*> m_sliders;
	QVBoxLayout* m_layout;
};

#endif // MULTISLIDERWIDGET_H
