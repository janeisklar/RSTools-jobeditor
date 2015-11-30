#include "SettingWidget.h"
#include <QPushButton>
#include <QBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSpacerItem>
//#include <QSpinBox>
//#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <glib.h>

using namespace rstools::batch::util;

SettingWidget::SettingWidget(RSTask* task, rsUIOption *option, QWidget *parent) : QGroupBox(parent)
{
    this->task   = task;
    this->option = option;
    setupLayout();
}

SettingWidget::~SettingWidget()
{
    
}

rsUIOption* SettingWidget::getSetting()
{
    return option;
}

void SettingWidget::setupLayout()
{       
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
    
    setTitle(QString(option->name));

    QLabel *description = new QLabel();
    description->setText(QString(
        option->gui_description == NULL
        ? option->cli_description
        : option->gui_description 
    ));
    description->setWordWrap(true);
    layout->addWidget(description);
        
    createValueWidget();
    layout->addWidget(valueWidget);
    
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setLayout(layout);
}

void SettingWidget::createValueWidget()
{
    rsArgument* argument = task->getArgument(option->name);
    
    switch(option->type) {
        case G_OPTION_ARG_FILENAME:
        case G_OPTION_ARG_STRING:
        case G_OPTION_ARG_STRING_ARRAY:
        case G_OPTION_ARG_CALLBACK:
        case G_OPTION_ARG_INT:
        case G_OPTION_ARG_INT64:
        case G_OPTION_ARG_DOUBLE:
            {
                // Display text box if number of values is not restricted
                if ( option->allowedValues == NULL ) {
                    if ( option->nLines < 2 ) {
                        QLineEdit *w = new QLineEdit();
                        valueWidget = w;
                        w->setPlaceholderText(option->cli_arg_description);
                        connect(w, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));
                        if ( argument != NULL ) {
                            w->setText(argument->value);
                        } else if ( option->defaultValue != NULL ) {
                            w->setText(option->defaultValue);
                        }
                    } else { // create a QTextEdit field instead
                        QPlainTextEdit *w = new QPlainTextEdit();
                        valueWidget = w;
                        connect(w, SIGNAL(textChanged()), this, SLOT(textChanged()));
                        if ( argument != NULL ) {
                            w->setPlainText(argument->value);
                        } else if ( option->defaultValue != NULL ) {
                            w->setPlainText(option->defaultValue);
                        }
                        QFontMetrics m(w->font()) ;
                        int rowHeight = m.lineSpacing() ;
                        w->setFixedHeight(option->nLines * rowHeight) ;
                        w->setLineWrapMode(QPlainTextEdit::NoWrap);
                    }
                } else { // if the allowed values are restricted display radio buttons instead
                    QWidget *w = new QWidget();
                    QBoxLayout *wLayout = new QBoxLayout(QBoxLayout::TopToBottom);
                    QButtonGroup *buttonGroup = new QButtonGroup();
                    buttonGroup->setExclusive(true);
                    valueWidget = w;
                    connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(buttonClicked(int)));
                    
                    // go through all options and add a radio button for them
                    rsUIOptionValue** values = option->allowedValues;
                    for (size_t i=0; values[i] != NULL; i++ ) {
                        // add radio button
                        QRadioButton *b = new QRadioButton(QString("'")+QString(values[i]->name)+QString("'"));
                        QFont f("Arial", 12, QFont::Bold);
                        b->setFont(f);
                        buttonGroup->addButton(b, (int)i);
                        wLayout->addWidget(b);
                        
                        // set it to checked if it is the default or set value
                        b->setChecked(false);
                        if ( argument != NULL ) {
                            if ( ! strcmp(argument->value,values[i]->name) ) {
                                b->setChecked(true);
                            }
                        } else if ( ! strcmp(option->defaultValue,values[i]->name) ) {
                            b->setChecked(true);
                        }
                        
                        // add its description
                        QLabel *label = new QLabel(values[i]->description);
                        label->setIndent(22);
                        label->setWordWrap(true);
                        label->setContentsMargins(0, 0, 0, 4);
                        QFont f2("Arial", 11, QFont::Normal);
                        label->setFont(f2);
                        wLayout->addWidget(label);
                    }
                    w->setLayout(wLayout);
                }
            }
            break;
        /*
        case G_OPTION_ARG_INT:
        case G_OPTION_ARG_INT64:
            valueWidget = new QSpinBox();
            break;
        case G_OPTION_ARG_DOUBLE:
            valueWidget = new QDoubleSpinBox();
            break;
        */
        case G_OPTION_ARG_NONE:
            {
                QCheckBox *w = new QCheckBox("Enabled"); // new SwitchWidget();
                valueWidget = w;
                connect(w, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
                if ( argument != NULL ) {
                    w->setCheckState(Qt::Checked);
                } else {
                    w->setCheckState(Qt::Unchecked);
                }
            }
            break;
        default:
            throw std::invalid_argument("UI argument type unknown");
    }
}

// Slot for QLineEdits
void SettingWidget::textChanged(QString newValue)
{
    QByteArray ba = newValue.toLatin1();
    const char *newValueCString = ba.data();
    rsArgument* argument = task->getArgument(option->name);
        
    char *newV = (char*)rsMalloc(sizeof(char)*(strlen(newValueCString)+1));
    sprintf(newV, "%s", newValueCString);
    
    if ( argument != NULL ) {
        char *oldV = argument->value;
        argument->value = newV;
        rsFree(oldV);
    } else {
        argument = (rsArgument*)rsMalloc(sizeof(rsArgument));
        
        char *name = (char*)rsMalloc(sizeof(char)*(strlen(option->name)+1));
        sprintf(name, "%s", option->name);
        
        argument->key = name;
        argument->value = newV;
        
        task->addArgument(argument);
    }
}

// Slot for QPlainTextEdit
void SettingWidget::textChanged()
{
    const QString s = ((QPlainTextEdit*)valueWidget)->toPlainText();
    textChanged(s);
}

// Slot for QButtonGroup
void SettingWidget::buttonClicked(int id)
{
    rsUIOptionValue** values = option->allowedValues;
    const char *value = values[id]->name;
    const QString s = QString::fromLatin1(value);
    
    textChanged(s);
}

// Slot for QCheckBox
void SettingWidget::stateChanged(int state)
{
    if ( state == Qt::Checked ) {
        rsArgument* argument = task->getArgument(option->name);
        if ( argument == NULL ) {
            argument = (rsArgument*)rsMalloc(sizeof(rsArgument));
            
            argument->key = (char*)rsMalloc(sizeof(char)*(strlen(option->name)+1));
            sprintf(argument->key, "%s", option->name);
            argument->value = NULL;
            
            task->addArgument(argument);
        }
    } else {
        rsArgument* argument = task->getArgument(option->name);
        if ( argument != NULL ) {
            task->removeArgument(option->name);
            //rsFree(argument);
        }
    }
}
