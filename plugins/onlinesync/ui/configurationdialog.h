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
        ConfigurationDialog( QWidget *parent=0 );
        ~ConfigurationDialog();

    private:
        Ui::ConfigurationDialog ui;

    private slots:
        void slotButtonAddClicked();
};
 
}

#endif
