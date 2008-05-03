#ifndef UI_CONFIGURATIONDIALOG_H
#define UI_CONFIGURATIONDIALOG_H
namespace Ui 
{
    class ConfigurationDialog {};
}
#endif

#ifndef CONFIGURATIONFRAME_H
#define CONFIGURATIONFRAME_H
 
#include <KDialog>

namespace feedsync
{

class ConfigurationDialog : public KDialog
{
    Q_OBJECT
    public:
        // Constructor if the dialog must be created
        ConfigurationDialog( QWidget *parent=0 );
        // Constructor if the dialog must not be created
        ConfigurationDialog( Ui::ConfigurationDialog * configUi , QWidget * parent );
        ~ConfigurationDialog();

    private:
        Ui::ConfigurationDialog * ui;
        QWidget * widget;

    private Q_SLOTS:
        virtual void slotButtonClicked(int button);
        void slotButtonAddClicked();
        void slotButtonUpdateClicked();
        void slotButtonRemoveClicked();
        void slotDelayedInit();
        void slotFinished();
        void refresh();
};
 
}

#endif
