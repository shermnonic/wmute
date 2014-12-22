#ifndef SHADERCODEEDITOR_H
#define SHADERCODEEDITOR_H

#include <QWidget>
#include <QString>

class QAction;
class CodeEditor;
class ShaderModule;

class ShaderEditorWidget : public QWidget
{
	Q_OBJECT

public:
	ShaderEditorWidget( QWidget* parent=0 );
	
	void setShaderModule( ShaderModule* sm );

protected slots:
	void resetShader();
	void updateShader();

protected:
	void setDirty( bool b );

	QString getShaderSource() const;

private:
	CodeEditor*   m_codeEditor;
	ShaderModule* m_shaderModule;
	QAction      *m_actReset,
		         *m_actUpdate;
	bool          m_dirty;
	QString       m_cache;
};

#endif // SHADERCODEEDITOR_H
