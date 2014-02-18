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

	/// Set number of shown sliders
	void setNumberOfSliders( int num );

	///@{ Set slider value(s)
	void setValue( int slider_idx, int value );
	void setValues( QVector<int> values );
	void setValues( int same_value_for_all );
	///@}

	///@{ Set slider min/max value range(s)
	void setRange( int slider_idx, int min_range, int max_range );
	void setRange( int min_range, int max_range );
	///@}

	/// Set integer slider values from an array of double values.
	/// A double value D is converted via I=(int)((D+shift)*scale) to integer.
	void getValuesAsDouble( std::vector<double>& values, double shift, double scale ) const;

protected slots:
	void onValueChange( int slider_idx );	

protected:
	// Note: Currently it is only safe to call createSliders() once! Multiple
	//       calls somehow lead to a performance issue in our application.
	void createSliders( int num );
	
private:
	QSignalMapper* m_sigmap;
	QVector<QSlider*> m_sliders;
	QVBoxLayout* m_layout;
	int m_numActiveSliders;
};

#endif // MULTISLIDERWIDGET_H
