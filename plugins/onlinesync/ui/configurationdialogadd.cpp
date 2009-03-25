#include "configurationdialogadd.h"

#include <kdebug.h>
#include <QErrorMessage>
#include <QTimer>


using namespace feedsync;

ConfigurationDialogAdd::ConfigurationDialogAdd( QWidget *parent) : KDialog(parent)
{
    kDebug();

    // UI setup
    QWidget *widget = new QWidget( parent );
    ui.setupUi(widget);
    setMainWidget( widget );

    setWindowTitle( i18n("Modify Online Reader Account") );
    ui.cb_AggregatorType->addItem( i18n("Google Reader") , QVariant("GoogleReader") );
    /* TODO OPML not available for the first version
    ui.cb_AggregatorType->addItem( i18n("OPML file") , QVariant("Opml") ); */


    slotUpdateUI();

    connect( ui.cb_AggregatorType, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotUpdateUI() ) );
}

ConfigurationDialogAdd::~ConfigurationDialogAdd()
{
    kDebug();
}

void ConfigurationDialogAdd::accept()
{
    kDebug();

    if (ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) == "GoogleReader") {

        if (ui.le_loginGoogleReader->text()=="")
            return;
        if (ui.le_passwdGoogleReader->text()=="")
            return;
        // Remove old
        if (_baseconfigname != "") {
            KConfig config("akregator_feedsyncrc");
            config.deleteGroup(_baseconfigname);
        }
        // Insert new
        KConfig config("akregator_feedsyncrc");
        KConfigGroup generalGroup( &config, "FeedSyncSource_GoogleReader" + ui.le_loginGoogleReader->text() );
        generalGroup.writeEntry( "AggregatorType", ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) );
        generalGroup.writeEntry( "Login", ui.le_loginGoogleReader->text() );
        generalGroup.writeEntry( "Password", ui.le_passwdGoogleReader->text() );
        generalGroup.writeEntry( "Identifier", ui.le_loginGoogleReader->text() );
        generalGroup.config()->sync();
    } else if (ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) == "Opml") {

        if (ui.filerequester->url().isEmpty() )
            return;
        // Remove old
        if (_baseconfigname != "") {
            KConfig config("akregator_feedsyncrc");
            config.deleteGroup(_baseconfigname);
        }
        // Insert new
        KConfig config("akregator_feedsyncrc");
        const KUrl url = ui.filerequester->url();
        KConfigGroup generalGroup( &config, "FeedSyncSource_Opml" + url.url() );
        generalGroup.writeEntry( "AggregatorType", ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) );
        generalGroup.writeEntry( "Filename", url.url() );
        generalGroup.writeEntry( "Identifier", url.url() );
        generalGroup.config()->sync();
    }
    done( KDialog::Ok );
}

void ConfigurationDialogAdd::load( const KConfigGroup& group )
{
    kDebug();
    _baseconfigname = group.name();

    ui.cb_AggregatorType->setCurrentIndex( ui.cb_AggregatorType->findData( group.readEntry( "AggregatorType", QString() ) ) );

    if ( group.readEntry( "AggregatorType", QString() ) == "GoogleReader") {

        ui.le_loginGoogleReader->setText( group.readEntry( "Login", QString() ) );
        ui.le_passwdGoogleReader->setText( group.readEntry( "Password", QString() ) );

    } else if ( group.readEntry( "AggregatorType", QString() ) == "Opml") {

        ui.filerequester->setUrl( group.readEntry( "Filename", QString() ) );

    } else {

    }
}

// SLOT

void ConfigurationDialogAdd::slotUpdateUI()
{
    kDebug();

    if (ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) == "GoogleReader") {
        ui.groupOpml->hide();
        ui.groupGoogleReader->show();

    } else if (ui.cb_AggregatorType->itemData( ui.cb_AggregatorType->currentIndex() ) == "Opml") {
        ui.groupGoogleReader->hide();
        ui.groupOpml->show();

    } else {
    }
}

#include "configurationdialogadd.moc"
