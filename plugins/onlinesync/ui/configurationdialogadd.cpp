#include "ui_configurationdialogadd.h"
#include "configurationdialogadd.h"

#include <kdebug.h>

namespace feedsync
{

ConfigurationDialogAdd::ConfigurationDialogAdd( QWidget *parent)
{
    kDebug();

    // UI setup
    QWidget *widget = new QWidget( this );
    ui.setupUi(widget);
    setMainWidget( widget );
    setCaption( i18n("Online reader Add") );

    // Signal
   connect( this, SIGNAL( okClicked() ), this, SLOT( slotButtonOkClicked() ) );
    // setButtons( KDialog::Close );
}

ConfigurationDialogAdd::~ConfigurationDialogAdd()
{
    kDebug();
}

void ConfigurationDialogAdd::slotButtonOkClicked()
{
    kDebug();
    // Save into config file
    KConfig config("akregator_feedsyncrc");
    KConfigGroup generalGroup( &config, "FeedSyncSource_" + ui.le_account->text() );
    generalGroup.writeEntry( "Account", ui.le_account->text() );
    generalGroup.writeEntry( "Password", ui.le_passwd->text() );
    generalGroup.config()->sync();
}

}
