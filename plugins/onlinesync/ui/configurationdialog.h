#ifndef CONFIGURATIONWIDGET_H
#define CONFIGURATIONWIDGET_H

#include "ui_configurationwidget.h"

#include <KDialog>

namespace feedsync
{
class ConfigurationDialog : public KDialog
{
    Q_OBJECT
    public:
        explicit ConfigurationDialog( QWidget *parent=0 );

    public Q_SLOTS:
        /* reimp */ void accept();
        
    private:
        Ui::ConfigurationWidget* m_widget;
};
 
}

#endif
