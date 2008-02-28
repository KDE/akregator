#include "ui_configurationdialog.h"
#include "configurationdialog.h"
#include "configurationdialogadd.h"

#include <kdebug.h>

namespace feedsync
{

ConfigurationDialog::ConfigurationDialog( QWidget *parent)
{
    kDebug();

    // UI setup
    QWidget *widget = new QWidget( this );
    ui.setupUi(widget);
    setMainWidget( widget );
    setCaption( i18n("Online reader") );

    // Init
    ui.list_readerList->setColumnCount(2);
    QStringList deleteTags;
        deleteTags.append( i18n("Never") );
        deleteTags.append( i18n("Always") );
        deleteTags.append( i18n("Ask") );
    ui.cb_deleteFeeds->addItems(deleteTags);
    QStringList title;
        title.append( i18n("Type") );
        title.append( i18n("Description") );
    ui.list_readerList->setHeaderLabels(title);

    // Slots
    connect( ui.b_add, SIGNAL( clicked() ), this, SLOT( slotButtonAddClicked() ) );

    // Read configuration
    KConfig config("akregator_feedsyncrc");
    foreach ( const QString& groupname, config.groupList() ) {
        if (groupname.left(15)=="FeedSyncSource_") {
            kDebug() << groupname;
            KConfigGroup generalGroup( &config, groupname );
            QList<QTreeWidgetItem *> items;
            QStringList line;
                line.append("Google Reader");
                line.append( generalGroup.readEntry( "Account", QString() ) );
            items.append(new QTreeWidgetItem((QTreeWidget*)0,line));
            ui.list_readerList->insertTopLevelItems(0, items);
        }
    }
}

ConfigurationDialog::~ConfigurationDialog()
{
    kDebug();
}

void ConfigurationDialog::slotButtonAddClicked()
{
    kDebug();
    ConfigurationDialogAdd * addDlg = new ConfigurationDialogAdd();
    addDlg->show();
}

}
