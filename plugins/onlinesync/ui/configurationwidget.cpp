#include "configurationwidget.h"
#include "configurationdialogadd.h"

#include <kdebug.h>
#include <QPointer>
#include <QTimer>

using namespace feedsync;

ConfigurationWidget::ConfigurationWidget( QWidget *parent ) : QWidget( parent )
{
    kDebug();

    ui.setupUi( this );

    refresh();
    
    // Init
    ui.cb_deleteFeeds->addItem( i18n("Nothing") , QVariant("Nothing") );
    ui.cb_deleteFeeds->addItem( i18n("Categories") , QVariant("Category") );
    ui.cb_deleteFeeds->addItem( i18n("Feeds") , QVariant("Feed") );
    ui.cb_deleteFeeds->addItem( i18n("Ask") , QVariant("Ask") );

    // Read config
    KConfig config("akregator_feedsyncrc");
    KConfigGroup generalGroup( &config, "FeedSyncConfig" );
    if (ui.cb_deleteFeeds->findData( QVariant( generalGroup.readEntry( "RemovalPolicy", QString() ) ) ) < 0) {
        ui.cb_deleteFeeds->setCurrentIndex( 0 );
    } else {
        ui.cb_deleteFeeds->setCurrentIndex( ui.cb_deleteFeeds->findData( QVariant(generalGroup.readEntry( "RemovalPolicy", QString() ) ) ) );
    }

    // Slots
    connect( ui.b_add, SIGNAL( clicked() ), this, SLOT( slotButtonAddClicked() ) );
    connect( ui.b_update, SIGNAL( clicked() ), this, SLOT( slotButtonUpdateClicked() ) );
    connect( ui.b_remove, SIGNAL( clicked() ), this, SLOT( slotButtonRemoveClicked() ) );
}


ConfigurationWidget::~ConfigurationWidget()
{
    kDebug();
}

void ConfigurationWidget::refresh()
{
    load();
}

void ConfigurationWidget::load()
{
    kDebug();

    // Clear
    ui.list_readerList->clear();

    // Read configuration
    KConfig config("akregator_feedsyncrc");
    QList<QTreeWidgetItem *> items;
    foreach ( const QString& groupname, config.groupList() ) {
        if (groupname.left(15)=="FeedSyncSource_") {
            kDebug() << groupname;
            KConfigGroup generalGroup( &config, groupname );
            QStringList line;
            line.append( generalGroup.readEntry( "AggregatorType", QString() ) );
            line.append( generalGroup.readEntry( "Identifier", QString() ) );
            line.append( groupname );
            items.append( new QTreeWidgetItem((QTreeWidget*)0,line) );
        }
        ui.list_readerList->insertTopLevelItems(0, items);
    }
}

// SLOT

void ConfigurationWidget::slotButtonUpdateClicked()
{
    kDebug();
    const QList<QTreeWidgetItem *> m_items = ui.list_readerList->selectedItems();
    if (m_items.isEmpty() )
        return;
    kDebug() << m_items.at(0)->text(2);
    KConfig config("akregator_feedsyncrc");
    KConfigGroup configgroup( &config, m_items.at(0)->text(2) );
    QPointer<ConfigurationDialogAdd> dlg = new ConfigurationDialogAdd( this );
    dlg->load( configgroup );
    if ( dlg->exec() == KDialog::Ok )
         refresh();
    delete dlg;
}

void ConfigurationWidget::slotButtonAddClicked()
{
    kDebug();
    QPointer<ConfigurationDialogAdd> addDlg = new ConfigurationDialogAdd( this );
    if ( addDlg->exec() == KDialog::Ok )
        refresh();
    delete addDlg;
}

void ConfigurationWidget::slotButtonRemoveClicked()
{
    // kDebug();

    QList<QTreeWidgetItem *> m_items = ui.list_readerList->selectedItems();
    if ( !m_items.isEmpty() ) {
        kDebug() << m_items.at(0)->text(2);
        KConfig config("akregator_feedsyncrc");
        config.deleteGroup(m_items.at(0)->text(2));
    }

    refresh();
}

void ConfigurationWidget::save() {
    kDebug();
    // Save the removal policy
    KConfig config("akregator_feedsyncrc");
    KConfigGroup generalGroup( &config, "FeedSyncConfig" );
    generalGroup.writeEntry( "RemovalPolicy", ui.cb_deleteFeeds->itemData( ui.cb_deleteFeeds->currentIndex() ) );
    generalGroup.config()->sync();
}


#include "configurationwidget.moc"
