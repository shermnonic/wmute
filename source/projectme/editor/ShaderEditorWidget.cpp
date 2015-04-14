#include "ShaderEditorWidget.h"
#include "CodeEditor.h"
#include "ShaderModule.h"
#include "Highlighter.h"
#include <QToolBar>
#include <QVBoxLayout>
#include <QAction>
#include <QFileDialog>

ShaderEditorWidget::ShaderEditorWidget( QWidget* parent )
: QWidget( parent ),
  m_dirty( false )
{
	// Setup editor 

	m_codeEditor = new CodeEditor();

	QFont font("Consolas");
	font.setStyleHint( QFont::Monospace );
	font.setFixedPitch( true );
    font.setPointSize( 8 );
	m_codeEditor->document()->setDefaultFont( font );	
	
	m_codeEditor->setLineWrapMode( QPlainTextEdit::NoWrap );
	m_codeEditor->setTabStopWidth( 4 * QFontMetrics(font).averageCharWidth() );

	Highlighter* highlighter = new Highlighter( m_codeEditor->document() );
	
	// GUI

	m_actReset  = new QAction( tr("Reset"), this );
	m_actUpdate = new QAction( tr("Update"), this );
	m_actUpdate->setShortcut( tr("Ctrl+U") );

	QAction* actExport = new QAction( tr("Export"), this );
	
	QToolBar* toolbar = new QToolBar( tr("Actions"), this );
	toolbar->addAction( m_actReset );
	toolbar->addAction( m_actUpdate );	
	toolbar->addAction( actExport );
	toolbar->setContentsMargins( 0,0,0,0 );
	
	QVBoxLayout* l = new QVBoxLayout();
	l->addWidget( toolbar );
	l->addWidget( m_codeEditor );
	l->setContentsMargins( 0,0,0,0 );
	setLayout( l );
	
	connect( m_actReset,  SIGNAL(triggered()), this, SLOT(resetShader()) );
	connect( m_actUpdate, SIGNAL(triggered()), this, SLOT(updateShader()) );
	connect( actExport,   SIGNAL(triggered()), this, SLOT(exportShader()) );
}

void ShaderEditorWidget::setDirty( bool b )
{
	m_dirty = b;
}

void ShaderEditorWidget::setShaderModule( ShaderModule* sm )
{
	m_shaderModule = sm;
	QString name = sm ? QString::fromStdString( sm->getName() ) : "";
	setWindowTitle( tr("Editor %1").arg( name  ) );	
	m_cache = getShaderSource();
	resetShader();
}

void ShaderEditorWidget::resetShader()
{
	m_codeEditor->document()->setPlainText( m_cache );
	setDirty( false );
}

void ShaderEditorWidget::updateShader()
{
	if( !m_shaderModule )
		return;
	
	QString text = m_codeEditor->document()->toPlainText();
	if( m_shaderModule->setShaderSource( text.toStdString() ) )
	{
		setDirty( false );
		emit shaderUpdated((ModuleBase*)m_shaderModule);
	}
}

void ShaderEditorWidget::exportShader()
{
	if( !m_shaderModule )
		return;

	QString filename = QFileDialog::getSaveFileName( this, tr("Save shader source"),
		"", tr("Shader source file (*.fs)") );

	if( filename.isEmpty() )
		return;

	m_shaderModule->saveShader( filename.toStdString().c_str() );
}

QString ShaderEditorWidget::getShaderSource() const
{
	QString source;
	if( m_shaderModule )
		source = QString::fromStdString( m_shaderModule->getShaderSource() );
	return source;
}
