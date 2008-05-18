#include "ui_configurationdialogadd.h"
#include "configurationdialogadd.h"

#include <kdebug.h>
#include <QErrorMessage>
#include <QTimer>


namespace feedsync
{

ConfigurationDialogAdd::ConfigurationDialogAdd( QWidget *parent) : KDialog(parent)
{
    kDebug();

    // UI setup
    QWidget *widget = new QWidget( parent );
    ui = new Ui::ConfigurationDialogAdd();
    ui->setupUi(widget);
    setMainWidget( widget );

    setCaption( i18n("Online Reader: Add") );
    ui->groupReaderType->setTitle( i18n("Type of Reader") );
    ui->groupGoogleReader->setTitle( i18n("Parameters") );
    ui->lAccountGoogleReader->setText( i18n("Account") );
    ui->lPasswordGoogleReader->setText( i18n("Password") );

    ui->cb_AggregatorType->addItem( i18n("Google Reader") , QVariant("GoogleReader") );
    /* TODO OPML not available for the first version
    ui->cb_AggregatorType->addItem( i18n("OPML file") , QVariant("Opml") ); */


    slotUpdateUI();

    connect( this, SIGNAL( finished() ), this, SLOT( slotFinished() ) );
    connect( ui->cb_AggregatorType, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotUpdateUI() ) );

    QTimer::singleShot( 0, this, SLOT(slotDelayedInit()) );
}

ConfigurationDialogAdd::~ConfigurationDialogAdd()
{
    kDebug();
    delete ui;
}

void ConfigurationDialogAdd::check()
{
    kDebug();

    if (ui->cb_AggregatorType->itemData( ui->cb_AggregatorType->currentIndex() ) == "GoogleReader") {

        if (ui->le_loginGoogleReader->text()=="") {
            return;
        } else if (ui->le_passwdGoogleReader->text()=="") {
            return;
        } else {
            // Remove old
            if (_baseconfigname != "") {
                KConfig config("akregator_feedsyncrc");
                config.deleteGroup(_baseconfigname);
            }
            // Insert new
            KConfig config("akregator_feedsyncrc");
            KConfigGroup generalGroup( &config, "FeedSyncSource_GoogleReader" + ui->le_loginGoogleReader->text() );
            generalGroup.writeEntry( "AggregatorType", ui->cb_AggregatorType->itemData( ui->cb_AggregatorType->currentIndex() ) );
            generalGroup.writeEntry( "Login", ui->le_loginGoogleReader->text() );
            generalGroup.writeEntry( "Password", ui->le_passwdGoogleReader->text() );
            generalGroup.writeEntry( "Identifier", ui->le_loginGoogleReader->text() );
            generalGroup.config()->sync();
            // Valid
            accept();
        }

    } else if (ui->cb_AggregatorType->itemData( ui->cb_AggregatorType->currentIndex() ) == "Opml") {

        if (ui->le_filenameOpml->text()=="") {
            return;
        } else {
            // Remove old
            if (_baseconfigname != "") {
                KConfig config("akregator_feedsyncrc");
                config.deleteGroup(_baseconfigname);
            }
            // Insert new
            KConfig config("akregator_feedsyncrc");
            KConfigGroup generalGroup( &config, "FeedSyncSource_Opml" + ui->le_filenameOpml->text() );
            generalGroup.writeEntry( "AggregatorType", ui->cb_AggregatorType->itemData( ui->cb_AggregatorType->currentIndex() ) );
            generalGroup.writeEntry( "Filename", ui->le_filenameOpml->text() );
            generalGroup.writeEntry( "Identifier", ui->le_filenameOpml->text() );
            generalGroup.config()->sync();
            // Valid
            accept();
        }

    } else {
    }
}

void ConfigurationDialogAdd::load( const KConfigGroup& group )
{
    kDebug();
    _baseconfigname = group.name();

    ui->cb_AggregatorType->setCurrentIndex( ui->cb_AggregatorType->findData( group.readEntry( "AggregatorType", QString() ) ) );

    if ( group.readEntry( "AggregatorType", QString() ) == "GoogleReader") {

        ui->le_loginGoogleReader->setText( group.readEntry( "Login", QString() ) );
        ui->le_passwdGoogleReader->setText( group.readEntry( "Password", QString() ) );

    } else if ( group.readEntry( "AggregatorType", QString() ) == "Opml") {

        ui->le_filenameOpml->setText( group.readEntry( "Filename", QString() ) );

    } else {

    }
}

// SLOT

void ConfigurationDialogAdd::slotDelayedInit()
{
    kDebug();
 }

void ConfigurationDialogAdd::slotButtonClicked(int button) {
    if (button == KDialog::Ok) {
        check();
    } else {
        KDialog::slotButtonClicked(button);
    }
}

void ConfigurationDialogAdd::slotFinished()
{
    kDebug();

    deleteLater();
}

void ConfigurationDialogAdd::slotUpdateUI()
{
    kDebug();

    if (ui->cb_AggregatorType->itemData( ui->cb_AggregatorType->currentIndex() ) == "GoogleReader") {
        ui->groupOpml->hide();
        ui->groupGoogleReader->show();

    } else if (ui->cb_AggregatorType->itemData( ui->cb_AggregatorType->currentIndex() ) == "Opml") {
        ui->groupGoogleReader->hide();
        ui->groupOpml->show();

    } else {
    }
}

}
