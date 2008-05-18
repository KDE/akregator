#ifndef CONFIGURATIONFRAMEADD_H
#define CONFIGURATIONFRAMEADD_H

#include <KDialog>

#include "ui_configurationdialogadd.h"


namespace feedsync
{

class ConfigurationDialogAdd : public KDialog
{
    Q_OBJECT
    public:
        explicit ConfigurationDialogAdd( QWidget *parent=0 );
        ~ConfigurationDialogAdd();
        void load( const KConfigGroup &group );

    public Q_SLOTS:
        /* reimp */ void accept();

    private:
        Ui::ConfigurationDialogAdd ui;
        QString _baseconfigname;

    protected Q_SLOTS:
        void slotUpdateUI();
};
 
}

#endif
