#pragma once
// QAutoGUI originally developed by Andrea Tagliasacchi 
// https://code.google.com/p/qautogui/
// and released under LGPL.
// This version here is based on qautogui-8c5c77372e3d from above git repository
// and contains some adaptions by Max Hermann from Jan 2015.
#include <QList>
#include <QHash>
#include <QFormLayout>
#include <QFrame>
#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QToolBox>
#include <QDebug>
#include <QComboBox>

#include <limits>

namespace QAutoGUI {

/// @brief The generic (pure virtual) superclass for all parameters
/// @todo use "#ifdef QT_GUI_LIB" to remove GUI code from this class
class Parameter : public QObject{
    friend class Parameters;
protected:
    QString  _label;     
    /// GUI initialization must be done in constructor
    Parameter(QString label) : _label(label){}
    virtual ~Parameter(){}
public:
    /// The name of the parameter
    virtual QString& label(){ return _label; }
    /// Prints parameter to a string
    virtual QString toString() = 0;
    /// Return a reference to a an *already* allocated widget
    /// Typically allocate it in the parameter constructor
    virtual QWidget* widget() = 0;
    /// Transfer data from "value" to "widget"
    virtual void update_gui() = 0; 
    /// Transfer data from "widget" to "value"
    virtual void collect_gui() = 0;
};

/// @brief this is a collection of parameters
class Parameters : public QObject{
protected:
    friend class ParametersWidget;
    friend class ParameterGroup;
    
    /// stores parameters in order of insertion
    QList<Parameter*> _list;

public:      
    /// @brief parameters must be parented
    Parameters(QObject* parent) : QObject(parent){}

	void clear() 
	{ 
		//qDeleteAll( _list.begin(), _list.end() );
		_list.clear(); 
	}
    
    /// @brief asks every parameter to read values from its GUI
    void collect_gui(){
        // qDebug() << "ParametersWidget::collect_gui";
        foreach(Parameter* par, _list)
            par->collect_gui();
    }
    
    /// @brief prints out a string representation of the parameters
    QString toString(QString prefix=""){
        QString retval;
        foreach(Parameter* par, _list){
            retval.append( prefix + par->toString() + "\n" );
        }
        return retval;
    }
    
    /// @brief adds a parameter to the collection
    template<class T>
    Parameters& operator<<(T& rhs){
        /// The collection steals the ownership
        rhs.setParent(this);
        /// Keep local list
        _list.push_back(&rhs);
        return *this;
    }
};

/** @brief Constructs a widget allowing the "live" editing of parameters. This 
 * contains a "form" and a "toolbox" of parameters. In the form we have a table 
 * where we have on left side the parameter "label",  on the right side, the 
 * parameter associated "widget" */
class ParametersWidget : public QWidget{
    Q_OBJECT
    Q_DISABLE_COPY(ParametersWidget)
private:
    QFormLayout* form; // set in constructor
    QToolBox* toolbox; // set in constructor
    Parameters* pars;  // set in load_parameters
public:
    /// @brief Collects the values of all parameters
    void collect_gui(){ if(pars!=NULL) pars->collect_gui(); }

public:   
    ParametersWidget(QWidget* parent = NULL) : QWidget(parent), pars(NULL){
        QVBoxLayout* layout = new QVBoxLayout(this);
        form = new QFormLayout(NULL);
        toolbox = new QToolBox(this);
        setLayout(layout);
        layout->addLayout(form);
        layout->addWidget(toolbox);
        
        /// Setup layouts
        form->setAlignment(Qt::AlignTop);
        form->setLabelAlignment(Qt::AlignLeft);
        form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    }

	void clear(){
		if( !pars ) return; // Sanity
		
		// Delete form row contents (not tested for groups!)
		// See http://stackoverflow.com/questions/13839952/how-to-simply-delete-row-in-qformlayout-programatically
        foreach(Parameter* par, pars->_list){            
            if(!par->label().isEmpty()) {
				form->labelForField( par->widget() )->deleteLater();
				par->widget()->deleteLater();
			}
            else
                par->widget()->deleteLater();   
        }
		form->update();
	}

    void load_parameters(Parameters& pars){
        this->pars = &pars;
        foreach(Parameter* par, pars._list){
            bool isgroup = ( strcmp( par->metaObject()->className(), "ParameterGroup" ) == 0 );
            
            /// Stuff not in groups are shared
            if(!isgroup){
                /// with empty labels, take all row!!
                if(!par->label().isEmpty()) 
                    form->addRow(par->label(),par->widget());
                else
                    form->addRow(par->widget());    
            }
            else {
                toolbox->addItem(par->widget(), par->label());
            }
        }
    }
};

/** @brief A dockable dialog window allowing live-editing of parameters.
 *  It offers an "Apply" and "Cancel" buttons to which you can build 
 *  connections with the given signals 
 */
class ParametersDialog : public QWidget{
    Q_OBJECT
    Q_DISABLE_COPY(ParametersDialog)
    
protected:
    ParametersWidget pWidget;
    QPushButton      applyButton;
    QPushButton      cancelButton;
    QVBoxLayout      wlayout; /// Core widget layout
    QHBoxLayout      hlayout; /// Horizontal layout to store Buttons

signals:
    void applyReleased();
    void cancelReleased();
    
public:
    ParametersDialog(Parameters& pars) 
        : applyButton("Apply",NULL),
          cancelButton("Cancel",NULL),
          wlayout(this),
          hlayout(NULL)
    {       
        pWidget.load_parameters(pars);
        
        /// Tweaks layouts to get a "tight" GUI
        wlayout.setMargin(0);  ///< frame padding
        hlayout.setMargin(0);  ///< frame padding
        hlayout.setSpacing(1); ///< button padding

        /// Cannot go below this
        this->setMinimumWidth(200);
        this->setMinimumHeight(100);       
        
        // Tell window to be as small as possible (doesn't work well, sets it to bigger than it could)
        // QSizePolicy policy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        // this->setSizePolicy(policy);

        /// Add items to layout
        wlayout.addWidget(&pWidget);
        wlayout.addLayout(&hlayout);
        hlayout.addWidget(&cancelButton);
        hlayout.addWidget(&applyButton);
               
        /// Connect button listener
        connect(&applyButton, SIGNAL(released()),this,SIGNAL(applyReleased()));
        connect(&cancelButton, SIGNAL(released()),this,SIGNAL(cancelReleased()));
    }
};

//----------------------------------------------------------------------------//
//
//
//
//                              SPECIALIZATIONS
//
//
//
//----------------------------------------------------------------------------//

/// @brief The specialized (with templates) superclass for all parameters. 
///        This template is simply used to reduce the amount of code needed. 
/// @internal Unfortunately, we cannot call the pure virtual methods in this context 
template< class _Value, class _Widget >
class TemplateParameter : public Parameter{
protected:
    typedef TemplateParameter<_Value, _Widget> Super;    
protected:
    _Value* _value;       ///< we receive a reference
    _Widget* _widget; ///< widget created to visualize the value
protected:
    /// For conveniency, the templated superclass (java style)
    TemplateParameter(QString label) : Parameter(label){
        this->_value = NULL;
        this->_widget = NULL; 
    }
    /// Constructor, reused by subclasses
    TemplateParameter(_Value& _value, QString label) : Parameter(label){ 
        this->_value = &_value;
        this->_widget = NULL; 
    }
    /// Return the address of the widget (do not allocate memory!!!)
    /// instead, perform the "new QWidget" in the constructor
    _Widget* widget(){ 
        if(_widget==NULL)
            _widget = new _Widget();
        return _widget; 
    }
    /// Returns a reference to the value stored
    _Value& value(){ return *_value; }
    /// @internal must be done in template, otherwise we don't know what a "value" is
    QString toString(){ return (_value==NULL) ? QString("Value '%1'=NULL").arg(_label) : QString("%1").arg(value()); }
};

/// @brief a static QLabel linked to a "QString"
class DescriptionBox : public TemplateParameter<QString, QLabel>{  
    Q_OBJECT
public:
    static DescriptionBox& New(QString value){ return *new DescriptionBox(value); }
public slots:
    void update_gui(){ widget()->setText(_text); }
    void collect_gui(){}
private:
    QString _text; ///< local instance
    DescriptionBox(QString text) : Super(""){
        _text = text;
        _value = &_text;
        widget()->setWordWrap(true); 
        update_gui();
    }
};

/// @brief a QCheckBox linked to a "bool"
class BooleanCheckbox : public TemplateParameter<bool, QCheckBox>{  
    Q_OBJECT
public:
    static BooleanCheckbox& New(bool& value, QString label){ return *new BooleanCheckbox(value, label); }
    QString toString() { return (value()==true)?"true":"false"; }
signals:
    void toggled(bool);
public:
    BooleanCheckbox& connect_to(const QObject* receiver, const char* signalname){
        connect( this, SIGNAL(toggled(bool)), receiver, signalname );
        return (*this);
    }
public slots:
    void update_gui(){ widget()->setChecked(value()); }
    void collect_gui(){ value() = widget()->isChecked(); }
private:
    BooleanCheckbox(bool& value, QString label) : Super(value, label){ 
        update_gui(); 
        connect( widget(), SIGNAL(clicked()), this, SLOT(collect_gui()));
        connect( widget(), SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
    }

};

/// @brief a QSpinBox linked to an "int"
class IntegerSpinEdit : public TemplateParameter<int, QSpinBox>{
    Q_OBJECT
public:
    static IntegerSpinEdit& New(int& value, QString label){ return *new IntegerSpinEdit(value, label); }
public slots:
    void update_gui(){ widget()->setValue( value() ); }
    void collect_gui(){ value() = widget()->value(); }
signals:
    void valueChanged(int);
public:
    IntegerSpinEdit& connect_to(const QObject* receiver, const char* signalname){
        connect( this, SIGNAL(valueChanged(int)), receiver, signalname);
        return (*this);
    }
public:
    IntegerSpinEdit& setRange(int min, int max){ 
        widget()->setRange(min, max);		
		widget()->setValue( value() );
        /// @todo value() to fit criteria above
        // collect_gui() ///< not working!!
        return *this; 
    }
private:
    IntegerSpinEdit(int& value, QString label) : Super(value, label){
        update_gui();
        /// makes it take a full line
        QSizePolicy policy = widget()->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        widget()->setSizePolicy(policy);
        /// connects
        connect( widget(), SIGNAL(valueChanged(int)), this, SLOT(collect_gui()) );
        connect( widget(), SIGNAL(valueChanged(int)), this, SIGNAL(valueChanged(int)) );
    }
};

/// @brief a QLineEdit linked to a "double"
class DoubleLineEdit : public TemplateParameter<double, QLineEdit>{
    Q_OBJECT
public:
    static DoubleLineEdit& New(double& value, QString label){ return *new DoubleLineEdit(value, label); }
signals:
    void valueChanged(double);
public:
    DoubleLineEdit& connect_to(const QObject* receiver, const char* signalname){
        connect( this, SIGNAL(valueChanged(double)), receiver, signalname);
        return (*this);
    }
public slots:
    void update_gui(){ widget()->setText( QString::number( value() ) ); }
    void collect_gui(){ 
        bool ok;
        value() = widget()->text().toDouble(&ok); 
        if(!ok)
            value() = std::numeric_limits<double>::quiet_NaN();
    }
private:
    DoubleLineEdit(double& value, QString label) : Super(value, label){ 
        update_gui(); 
        /// @todo allow only floats to be inserted (numbers and commas)
        // ....
        /// connects 
        connect( widget(), SIGNAL(textChanged(QString)), this, SLOT(collect_gui()) );
        connect( widget(), SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
    }
private slots:
    ///< converts a text change into a valueChange signal
    void textChanged(QString){ emit valueChanged( value() ); }
};

/// @brief something that can be used to group parameters, creating a tree-like structure
///        it is not reccomended to use more than 2 levels of the tree!!
class ParameterGroup : public Parameter{
    Q_OBJECT
    Q_DISABLE_COPY(ParameterGroup)
private:
    /// contained parameters
    Parameters inner;
    ParametersWidget* _widget;
    /// @todo store if the form is currently being displayed
    // bool isactivated;
private:
    ParameterGroup(QString label) : Parameter(label), inner(this), _widget(NULL){}
public:
    static ParameterGroup& New(QString label){ return *new ParameterGroup(label); }

    QString toString(){ 
        QString ret = label() + "\n";
        foreach(Parameter* par, inner._list)
            ret.append(par->toString() + "\n");
        return ret;
    }
        
public:
    ParametersWidget* widget(){ 
        if(_widget==NULL){
            _widget = new ParametersWidget(NULL);
            _widget->load_parameters(inner);
        }
        return _widget;
    }
public slots:
    void update_gui(){ 
        foreach(Parameter* par, inner._list)
            par->update_gui();
    }
    void collect_gui(){
        foreach(Parameter* par, inner._list)
            par->collect_gui();
    }
    
public:
    /// @brief adds a parameter to the collection
    template<class T>
    ParameterGroup& operator<<(T& rhs){ inner << rhs; return *this; }
};


//---- Stuff below added by Max ---

/// @brief a QCheckBox linked to a "bool" stored internally as int
class BooleanIntCheckbox : public TemplateParameter<int, QCheckBox>{  
    Q_OBJECT
public:
    static BooleanIntCheckbox& New(int& value, QString label){ return *new BooleanIntCheckbox(value, label); }
    QString toString() { return ((bool)value()==true)?"true":"false"; }
signals:
    void toggled(bool);
public:
    BooleanIntCheckbox& connect_to(const QObject* receiver, const char* signalname){
        connect( this, SIGNAL(toggled(bool)), receiver, signalname );
        return (*this);
    }
public slots:
    void update_gui(){ widget()->setChecked(value()); }
    void collect_gui(){ value() = widget()->isChecked(); }
private:
    BooleanIntCheckbox(int& value, QString label) : Super(value, label){ 
        update_gui(); 
        connect( widget(), SIGNAL(clicked()), this, SLOT(collect_gui()));
        connect( widget(), SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
    }
};


/// @brief a QSpinBox linked to an "double"
class DoubleSpinEdit : public TemplateParameter<double, QDoubleSpinBox>{
    Q_OBJECT
public:
    static DoubleSpinEdit& New(double& value, QString label){ return *new DoubleSpinEdit(value, label); }
public slots:
    void update_gui(){ widget()->setValue( value() ); }
    void collect_gui(){ value() = widget()->value(); }
signals:
    void valueChanged(double);
public:
    DoubleSpinEdit& connect_to(const QObject* receiver, const char* signalname){
        connect( this, SIGNAL(valueChanged(int)), receiver, signalname);
        return (*this);
    }
public:
    DoubleSpinEdit& setRange(double min, double max){ 
        widget()->setRange(min, max); 
		widget()->setValue( value() );
        /// @todo value() to fit criteria above
        // collect_gui() ///< not working!!
        return *this; 
    }
    DoubleSpinEdit& setDecimals(int prec){ 
        widget()->setDecimals(prec); 
		widget()->setValue( value() );
        return *this; 
    }
    DoubleSpinEdit& setSingleStep(double val){ 
        widget()->setSingleStep(val); 
        return *this; 
    }
private:
    DoubleSpinEdit(double& value, QString label) : Super(value, label){
        update_gui();
        /// makes it take a full line
        QSizePolicy policy = widget()->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        widget()->setSizePolicy(policy);
        /// connects
        connect( widget(), SIGNAL(valueChanged(double)), this, SLOT(collect_gui()) );
        connect( widget(), SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged(double)) );
    }
};


/// @brief a QComboBox linked to an "int"
class EnumCombobox : public TemplateParameter<int, QComboBox>{
    Q_OBJECT
public:
    static EnumCombobox& New(int& value, QString label){ return *new EnumCombobox(value, label); }
public slots:
    void update_gui(){ widget()->setCurrentIndex( value() ); }
    void collect_gui(){ value() = widget()->currentIndex(); }
signals:
    void valueChanged(int);
public:
    EnumCombobox& connect_to(const QObject* receiver, const char* signalname){
        connect( this, SIGNAL(currentIndexChanged(int)), receiver, signalname);
        return (*this);
    }
public:
	EnumCombobox& setItems( const QStringList& texts ){
		widget()->clear();
		widget()->addItems( texts );
		widget()->setCurrentIndex( value() );
        /// @todo value() to fit criteria above
        // collect_gui() ///< not working!!
		return *this;
	}
private:
    EnumCombobox(int& value, QString label) : Super(value, label){
        update_gui();
        /// makes it take a full line
        QSizePolicy policy = widget()->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        widget()->setSizePolicy(policy);
        /// connects
        connect( widget(), SIGNAL(currentIndexChanged(int)), this, SLOT(collect_gui()) );
        connect( widget(), SIGNAL(currentIndexChanged(int)), this, SIGNAL(valueChanged(int)) );
    }
};

} // namespace QAutoGUI
