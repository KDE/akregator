#ifndef UI_CONFIGURATIONDIALOGADD_H
#define UI_CONFIGURATIONDIALOGADD_H
namespace Ui 
{
    class ConfigurationDialogAdd {};
}
#endif

#ifndef CONFIGURATIONFRAMEADD_H
#define CONFIGURATIONFRAMEADD_H
 
#include <KDialog>


namespace feedsync
{

class ConfigurationDialogAdd : public KDialog
{
    Q_OBJECT
    public:
        explicit ConfigurationDialogAdd( QWidget *parent=0 );
        ~ConfigurationDialogAdd();
        void load( const KConfigGroup &group );

    private:
        Ui::ConfigurationDialogAdd * ui;
        void check();
        QString _baseconfigname;

    protected Q_SLOTS:
        virtual void slotButtonClicked(int button);
        void slotDelayedInit();
        void slotFinished();
        void slotUpdateUI();
};
 
}

#endif
