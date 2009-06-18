/*
 * This file is part of Akregator
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "migratefeedscommand.h"
#include "migratefeedswizard_p.h"
#include "importitemsjob.h"

#include <krss/importopmljob.h>
#include <krss/resource.h>
#include <krss/feedlist.h>
#include <krss/netfeed.h>
#include <krss/resourcemanager.h>

#include <akonadi/agentinstance.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agenttype.h>

#include <QComboBox>
#include <QFile>
#include <QHash>
#include <QLabel>
#include <QListView>
#include <QPointer>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QWidget>
#include <QWizard>
#include <QWizardPage>

#include <KDebug>
#include <KGlobal>
#include <KLocalizedString>
#include <KIcon>
#include <KStandardDirs>
#include <KUrl>

#include <boost/shared_ptr.hpp>

#include <cassert>

using namespace boost;
using namespace Akonadi;
using namespace Akregator;
using namespace KRss;

StartPage::StartPage( const QStringList& resources, QWidget* parent ) : QWizardPage( parent ), m_resourceBox( 0 ) {
    setTitle( i18n("Akonadi Data Migration" ) );
    QVBoxLayout* layout = new QVBoxLayout( this );
    QLabel* label = new QLabel( i18n("Since version 1.6, Akregator uses the Akonadi framework to store feeds and articles. Your feeds and articles from older Akregator versions will now be migrated." ) );
    label->setWordWrap( true );
    layout->addWidget( label );
    if ( !resources.isEmpty() ) {
        QLabel* resLabel = new QLabel( i18n("Please choose the resource to use for importing feeds and articles." ) );
        layout->addWidget( resLabel );
        QWidget* hbox = new QWidget;
        QHBoxLayout* hlayout = new QHBoxLayout( hbox );
        m_resourceBox = new QComboBox;
        Q_FOREACH( const QString& i, resources )
            m_resourceBox->addItem( i, i );
        m_resourceBox->setCurrentIndex( 0 );
        hlayout->addWidget( new QLabel( i18n("Resource:" ) ) );
        hlayout->addWidget( m_resourceBox );
        layout->addWidget( hbox );
    }
}

OpmlImportResultPage::OpmlImportResultPage( QWidget* parent ) : WizardPage( parent ) {
    QVBoxLayout* layout = new QVBoxLayout( this );
    m_progressBar = new QProgressBar;
    layout->addWidget( m_progressBar );
    m_resultLabel = new QLabel;
    m_resultLabel->setWordWrap( true );
    layout->addWidget( m_resultLabel );
}

ItemImportResultPage::ItemImportResultPage( QWidget* parent ) : WizardPage( parent ) {
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_progressBar = new QProgressBar );
    layout->addWidget( m_progressLabel = new QLabel );
    m_progressLabel->setWordWrap( true );
    m_model = new QStandardItemModel( this );
    QListView* listView = new QListView;
    layout->addWidget( listView );
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel( this );
    proxy->setSortRole( SortRole );
    proxy->setDynamicSortFilter( true );
    proxy->setSourceModel( m_model );
    listView->setModel( proxy );
}

void ItemImportResultPage::setState( ItemImportResultPage::State state, int current, int total ) {
    if ( state == Running ) {
        m_progressBar->setRange( 0, total );
        m_progressBar->setValue( current );
    } else {
        m_progressBar->setRange( 0, 100 );
        m_progressBar->setValue( state == Finished ? 100 : 0 );
    }
}

void OpmlImportResultPage::setState( OpmlImportResultPage::State state ) {
    m_progressBar->setRange( 0, state == Running ? 0 : 100 );
    m_progressBar->setValue( state == Finished ? 100 : 0 );
}

void ItemImportResultPage::itemImportFinished( const Akregator::ImportItemsJob* job, const QString& title ) {
    QStandardItem* item = new QStandardItem;
    item->setEditable( false );
    item->setData( job->error() != 0 ? Error : NoError );

    if ( job->error() ) {
        item->setText( i18n( "Failed: %1 (%2)", title, job->errorText() ) );
        item->setIcon( KIcon("dialog-error") );
    }
    else {
        item->setText( i18n( "Imported: %1", title ) );
        item->setIcon( KIcon("dialog-ok") );
    }
    m_model->appendRow( item );
}

class Akregator::MigrateFeedsCommand::Private {
    MigrateFeedsCommand* const q;
public:
    explicit Private( MigrateFeedsCommand* qq ) : q( qq ), opmlImportResultPageId( -1 ), itemImportResultPageId( -1 ) {
        opmlPath = KGlobal::dirs()->saveLocation("data", "akregator/data") + "feeds.opml";
    }

    ~Private() {
        delete wizard;
    }
    void doDoStart();
    void currentIdChanged( int id );
    void opmlImportFinished( KJob* job );

    void ensureWizardCreated();
    void ensureWizardShown();

    void determineResource();
    void startOpmlImport();
    void startNextItemImport();
    void itemImportFinished( KJob* );
    void resourceCreated( const QString& id );
    void resourceCreationFinished( KJob* );
    void wizardClosed();
    void feedListRetrievalFinished( KJob* );

    QPointer<QWizard> wizard;
    QPointer<OpmlImportResultPage> opmlImportResultPage;
    int opmlImportResultPageId;
    QPointer<ItemImportResultPage> itemImportResultPage;
    int itemImportResultPageId;
    QPointer<StartPage> startPage;
    shared_ptr<FeedList> feedList;
    QString opmlPath;
    QHash<QString, ImportOpmlJob::FeedInfo> feedsLeftToImport, allFeeds;
    QString resourceIdentifier;
};

MigrateFeedsCommand::MigrateFeedsCommand( QObject* parent ) : Command( parent ), d( new Private( this ) ) {
}

MigrateFeedsCommand::~MigrateFeedsCommand() {
    delete d;
}

void MigrateFeedsCommand::Private::wizardClosed() {
    q->emitResult();
}

void MigrateFeedsCommand::doStart() {
    QMetaObject::invokeMethod( this, "doDoStart" );
}

void MigrateFeedsCommand::Private::ensureWizardCreated() {
    if ( wizard )
        return;
    wizard = new QWizard( q->parentWidget() );
    wizard->setAttribute( Qt::WA_DeleteOnClose );
    connect( wizard, SIGNAL(destroyed(QObject*)), q, SLOT(wizardClosed()) );
    q->connect( wizard, SIGNAL(currentIdChanged(int)),
                 q, SLOT(currentIdChanged(int)) );
    wizard->setWindowTitle( i18n("Akonadi Data Migration" ) );

    const QStringList resIds = ResourceManager::self()->identifiers();

    startPage = new StartPage( resIds.size() > 1 ? resIds : QStringList() );
    startPage->setCommitPage( true );
    wizard->addPage( startPage );
    opmlImportResultPage = new OpmlImportResultPage;
    opmlImportResultPage->setCommitPage( true );
    opmlImportResultPage->setCompleted( false );
    opmlImportResultPageId = wizard->addPage( opmlImportResultPage );
    itemImportResultPage = new ItemImportResultPage;
    itemImportResultPageId = wizard->addPage( itemImportResultPage );
    itemImportResultPage->setCommitPage( true );
}

void MigrateFeedsCommand::Private::ensureWizardShown() {
    ensureWizardCreated();
    wizard->show();
}

void MigrateFeedsCommand::setOpmlFile( const QString& path ) {
    d->opmlPath = path;
}

void MigrateFeedsCommand::Private::doDoStart() {
    if ( !QFile::exists( opmlPath ) ) {
        q->emitResult();
        return;
    }

    //TODO: check "already migrated" setting

    ensureWizardShown();
}

void MigrateFeedsCommand::Private::resourceCreationFinished( KJob* j ) {
    const AgentInstanceCreateJob* const job = qobject_cast<const AgentInstanceCreateJob*>( j );
    assert( job );

    if ( job->error() ) {
        q->setError( MigrateFeedsCommand::ResourceCreationFailed );
        q->setErrorText( job->errorText() );
    }

    const AgentInstance instance = job->instance();

    resourceCreated( instance.identifier() );
}

void MigrateFeedsCommand::Private::resourceCreated( const QString& id ) {
    resourceIdentifier = id;
    assert( !resourceIdentifier.isEmpty() );
    RetrieveFeedListJob* job = new RetrieveFeedListJob( q );
    job->setResourceIdentifiers( QStringList() << resourceIdentifier );
    connect( job, SIGNAL(finished(KJob*)),
             q, SLOT(feedListRetrievalFinished(KJob*)) );
    job->start();
}

void MigrateFeedsCommand::Private::feedListRetrievalFinished( KJob* j ) {
    RetrieveFeedListJob* job = qobject_cast<RetrieveFeedListJob*>( j );
    assert( job );

    if ( job->error() ) {
        //TODO
        return;
    }

    feedList = job->feedList();
    startOpmlImport();
}

void MigrateFeedsCommand::Private::determineResource() {
    const QStringList ids = ResourceManager::self()->identifiers();

    if ( ids.size() == 1 )
        resourceIdentifier = ids.first();
    else if ( startPage->m_resourceBox )
        resourceIdentifier = startPage->m_resourceBox->itemData( startPage->m_resourceBox->currentIndex() ).toString();

    if ( !resourceIdentifier.isEmpty() ) {
        resourceCreated( resourceIdentifier );
        return;
    }

    const AgentType type = AgentManager::self()->type( "akonadi_opml_rss_resource" );
    AgentInstanceCreateJob* job = new AgentInstanceCreateJob( type );
    q->connect( job, SIGNAL(finished(KJob*)), q, SLOT(resourceCreationFinished(KJob*)) );
    job->start();
}

void MigrateFeedsCommand::Private::opmlImportFinished( KJob* j ) {
    const ImportOpmlJob* const job = qobject_cast<const ImportOpmlJob*>( j );
    assert( job );
    opmlImportResultPage->setState( OpmlImportResultPage::Finished );
    opmlImportResultPage->setCompleted( true );
    if ( job->error() ) {
        opmlImportResultPage->m_resultLabel->setText( i18n("<p>The import of the existing feed list failed:</p><p>%1</p><p>Could not import your feeds and archived articles.</p>", job->errorText() ) );
        return;
    }

    const QList<shared_ptr<const NetFeed> > netFeeds = feedList->constNetFeeds();
    Q_FOREACH( const shared_ptr<const NetFeed>& i, netFeeds ) {
        ImportOpmlJob::FeedInfo fi;
        fi.xmlUrl = i->xmlUrl();
        fi.title = i->title();
        allFeeds.insert( fi.xmlUrl, fi );
    }
    Q_FOREACH( const ImportOpmlJob::FeedInfo& fi, job->importedFeeds() )
        allFeeds.insert( fi.xmlUrl, fi );

    feedsLeftToImport = allFeeds;

    opmlImportResultPage->m_resultLabel->setText( i18n("<p>The existing feed list was successfully imported. In the next step, the archived articles from these feeds will be imported.</p>" ) );
}

void MigrateFeedsCommand::Private::startOpmlImport() {
    const Resource* const resource = ResourceManager::self()->resource( resourceIdentifier );
    assert( resource ); //TODO: remove assertion, handl error
    ImportOpmlJob* job = resource->createImportOpmlJob( KUrl::fromPath( opmlPath ) );
    connect( job, SIGNAL(finished(KJob*)), q, SLOT(opmlImportFinished(KJob*)) );
    job->start();
    opmlImportResultPage->setState( OpmlImportResultPage::Running );
    opmlImportResultPage->m_resultLabel->setText( i18n( "Importing feed list..." ) );
}

void MigrateFeedsCommand::Private::startNextItemImport() {
    if ( feedsLeftToImport.isEmpty() ) {
        itemImportResultPage->m_progressLabel->setText( i18n("Import of archived articles finished.") );
        itemImportResultPage->setState( ItemImportResultPage::Finished );
        itemImportResultPage->setCompleted( true );
        return;
    }

    const Resource* const resource = ResourceManager::self()->resource( resourceIdentifier );
    assert( resource ); //TODO: remove assertion, handl error

    const ImportOpmlJob::FeedInfo fi = feedsLeftToImport.take( feedsLeftToImport.keys().first() );
    Akregator::ImportItemsJob* job = new Akregator::ImportItemsJob( resource, q );
    q->connect( job, SIGNAL(finished(KJob*)), q, SLOT(itemImportFinished(KJob*)) );
    job->setXmlUrl( fi.xmlUrl );
    job->start();
    const int current = allFeeds.size() - feedsLeftToImport.size();
    itemImportResultPage->setState( ItemImportResultPage::Running, current, allFeeds.size() );
    itemImportResultPage->m_progressLabel->setText( i18nc( "current position, total number, feed title", "Importing items for feed %1 of %2: \"%3\"...", QString::number( current ), QString::number( allFeeds.size() ), fi.title ) );
}

void MigrateFeedsCommand::Private::itemImportFinished( KJob* j ) {
    const ImportItemsJob* const job = qobject_cast<const ImportItemsJob*>( j );
    assert( job );
    itemImportResultPage->itemImportFinished( job, allFeeds.value( job->xmlUrl() ).title );
    startNextItemImport();
}

void MigrateFeedsCommand::Private::currentIdChanged( int id ) {
    if ( id == opmlImportResultPageId ) {
        determineResource();
        return;
    }
    if ( id == itemImportResultPageId ) {
        startNextItemImport();
        return;
    }
}

#include "migratefeedscommand.moc"
#include "migratefeedswizard_p.moc"
