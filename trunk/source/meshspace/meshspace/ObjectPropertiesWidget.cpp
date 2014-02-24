// ObjectPropertiesWidget - scene::Object properties for SceneViewer
// Max Hermann, Jan 2014
#include "ObjectPropertiesWidget.h"
#include "MeshObject.h"
#include "PCAObject.h"
#include "TensorfieldObject.h"
#include "TensorfieldObjectWidget.h"
#include "MultiSliderWidget.h"

#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QTextEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QColor>
#include <QPixmap>
#include <QScrollArea>

//------------------------------------------------------------------------------
ObjectPropertiesWidget::ObjectPropertiesWidget( QWidget* parent )
: QWidget(parent)
{
	// -- Widgets
	
	QLabel* labelName = new QLabel(tr("Name"));
	m_leName = new QLineEdit(tr("(not set)"));
	labelName->setBuddy( m_leName );
	
	QLabel* labelColor = new QLabel(tr("Color"));
	m_pixmapColor = QPixmap(16,16);
	m_labelPixmapColor = new QLabel;
	m_labelPixmapColor->setPixmap( m_pixmapColor );
	labelColor->setBuddy( m_labelPixmapColor );
	
	QLabel* labelFrame = new QLabel(tr("Frame"));
	m_sliderFrame = new QSlider( Qt::Horizontal );
	m_sliderFrame->setRange(1,1);
	labelFrame->setBuddy( m_sliderFrame );
	
	QLabel* labelInfo = new QLabel(tr("Info"));
	m_teInfo = new	QTextEdit();	
	labelInfo->setBuddy( m_teInfo );

	m_pcaSlider = new MultiSliderWidget;
	m_pcaSliderArea = new QScrollArea;
	m_pcaSliderArea->setWidget( m_pcaSlider );
	m_pcaSliderArea->setWidgetResizable( true );

	m_tensorfieldObjectWidget = new TensorfieldObjectWidget;	
	m_tensorfieldObjectWidget->setVisible( false );

	// -- Layout

	QGridLayout* l1 = new QGridLayout();
	int row=0;
	l1->addWidget( labelName,  row,0 );
	l1->addWidget( m_leName,   row,1 ); row++;
	l1->addWidget( labelColor, row,0 );
	l1->addWidget( m_labelPixmapColor,  row,1 ); row++;
	
	QVBoxLayout* l2 = new QVBoxLayout();
	l2->addWidget( labelFrame );
	l2->addWidget( m_sliderFrame );
	
	QVBoxLayout* l3 = new QVBoxLayout();
	l3->addWidget( labelInfo );
	l3->addWidget( m_teInfo );	
	
	QVBoxLayout* layout = new QVBoxLayout();
	layout->addLayout( l1 );
	layout->addLayout( l2 );
	layout->addLayout( l3 );
	layout->addWidget( m_tensorfieldObjectWidget );
	layout->addWidget( m_pcaSliderArea );
	
	setLayout( layout );
	
	// -- Initialize
	reset();

	// -- Connections

	connect( m_sliderFrame, SIGNAL(valueChanged(int)), this, SLOT(changeFrame(int)) );
}

//------------------------------------------------------------------------------
void ObjectPropertiesWidget::reset()
{
	// Set pointer to NULL to avoid any updates caused by changing the UI
	m_obj = NULL;

	// Reset
	m_leName->setText( tr("(not set)") );
	setColor( scene::Color() );
	m_sliderFrame->setValue(1);
	m_sliderFrame->setRange(1,1);
	m_sliderFrame->setEnabled( false );
	m_teInfo->setText( tr("") );
	m_pcaSliderArea->setVisible( false );
	m_tensorfieldObjectWidget->setVisible( false );
	m_tensorfieldObjectWidget->setMaster( NULL );

	disconnect( m_pcaSlider, SIGNAL(valueChanged(int,int)), this, SLOT(changePCACoeff(int,int)) );
}

//------------------------------------------------------------------------------
void ObjectPropertiesWidget::setColor( scene::Color c )
{
	m_pixmapColor.fill( QColor((int)255.*c.r, (int)255.*c.g, (int)255.*c.b) );
	m_labelPixmapColor->setPixmap( m_pixmapColor );
}

//------------------------------------------------------------------------------
void ObjectPropertiesWidget::setSceneObject( scene::Object* obj )
{
	reset();
	
	// Disable widget if no object given
	if( !obj )
	{
		setEnabled( false );
		return;
	}
	
	// Fill in object properties	
	m_leName->setText( QString::fromStdString( obj->getName() ) );	
	setColor( obj->getColor() );
	
	//-----------------------------
	// TensorfieldObject specific controls
	//-----------------------------
	if( dynamic_cast<scene::TensorfieldObject*>(obj) )
	{
		scene::TensorfieldObject* tfo = dynamic_cast<scene::TensorfieldObject*>(obj);

		m_tensorfieldObjectWidget->setVisible( true );
		m_tensorfieldObjectWidget->setMaster( tfo );

		m_teInfo->setText(
			tr("Tensorfield object\n"
		       "#Glyphs = %1\n"
			   "#Glyph vertices = %2\n"
			   "#Glyph faces = %3\n")
			.arg( tfo->numGlyphs() )
			.arg( tfo->numGlyphVertices() ) 
			.arg( tfo->numGlyphFaces() )
		);
	}
	else
	//-----------------------------
	// PCAObject specific controls
	//-----------------------------
	if( dynamic_cast<scene::PCAObject*>(obj) )
	{
		scene::PCAObject* pco = dynamic_cast<scene::PCAObject*>(obj);
		
		// Sanity
		if( pco->numPCs() <= 0 )
			return;

		// Setup sliders
		m_pcaSlider->setNumberOfSliders( pco->numPCs() );
		m_pcaSlider->setRange( -300, 300 );
		m_pcaSlider->setValues( 0 );

		// Set slider values to current PC coefficients
		std::vector<double> coeffs;
		QVector<int> values;
		pco->getCoeffs( coeffs );
		for( int i=0; i < coeffs.size(); i++ )
			values.push_back( (int)(coeffs[i]*100.0) );
		m_pcaSlider->setValues( values );

		// Show sliders
		m_pcaSliderArea->setVisible( true );

		connect( m_pcaSlider, SIGNAL(valueChanged(int,int)), this, SLOT(changePCACoeff(int,int)) );

		m_teInfo->setText(
			tr("PCA object\n"
		       "#Vertices = %1\n"
			   "#Faces = %2\n"
			   "#Modes = %3\n")
			.arg( pco->numVertices() )
			.arg( pco->numFaces() ) 
			.arg( pco->numPCs() )
		);
	}
	else
	//-----------------------------
	// Mesh specific properties	
	//-----------------------------
	if( dynamic_cast<scene::MeshObject*>(obj) )
	{
		scene::MeshObject* meshobj = dynamic_cast<scene::MeshObject*>(obj);
		if( meshobj->isAnimation() )
		{
			m_sliderFrame->setRange( 1, meshobj->numFrames() );
			m_sliderFrame->setValue( meshobj->curFrame() );
			m_sliderFrame->setEnabled( true );
		}		

		m_teInfo->setText(
			tr("Mesh object\n"
		       "#Vertices = %1\n"
			   "#Faces = %2\n"
			   "#Frames = %3\n")
			.arg( meshobj->numVertices() )
			.arg( meshobj->numFaces() ) 
			.arg( meshobj->numFrames() )
		);
	}

	update();

	// Store pointer to currently selected object (e.g. for emitting signals)
	m_obj = obj;
}

//------------------------------------------------------------------------------
void ObjectPropertiesWidget::changeFrame( int frame )
{
	if( !m_obj )
		return;

	// Frames only supported by mesh objects
	scene::MeshObject* meshobj = dynamic_cast<scene::MeshObject*>( m_obj );
	if( meshobj )
	{
		meshobj->setFrame( frame-1 ); // 0-based index
		emit redrawRequired();
	}	
}

//------------------------------------------------------------------------------
void ObjectPropertiesWidget::changePCACoeff( int /*mode*/, int /*ivalue*/ )
{
	if( dynamic_cast<scene::PCAObject*>(m_obj) )
	{
		scene::PCAObject* pco = dynamic_cast<scene::PCAObject*>(m_obj);

		std::vector<double> coeffs;
		m_pcaSlider->getValuesAsDouble( coeffs, 0., 1./100. );

		pco->synthesize( coeffs );
		emit redrawRequired();
	}
}
