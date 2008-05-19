#ifndef CONFIGURATIONWIDGET_H
#define CONFIGURATIONWIDGET_H

#include "ui_configurationdialog.h"

#include <KDialog>

namespace feedsync
{

class ConfigurationWidget;

class ConfigurationDialog : public KDialog
{
    Q_OBJECT
    public:
        explicit ConfigurationDialog( QWidget *parent=0 );

    public Q_SLOTS:
        /* reimp */ void accept();
        
    private:
        ConfigurationWidget* m_widget;
};
 
}

#endif
