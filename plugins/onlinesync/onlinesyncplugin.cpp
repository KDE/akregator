/*
    This file is part of Akregator.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <KConfigGroup>


#include "feedsync.h"

#include "onlinesyncplugin.h"

#include <KActionCollection>
#include <KGenericFactory>
#include <KLocalizedString>

#include <QAction>

using namespace Akregator;

K_PLUGIN_FACTORY(OnlineSyncPluginFactory, registerPlugin<Akregator::OnlineSyncPlugin>();)
K_EXPORT_PLUGIN(OnlineSyncPluginFactory( "akregator_onlinesync_plugin" ) )

OnlineSyncPlugin::OnlineSyncPlugin( QObject* parent, const QVariantList& list ) : KParts::Plugin( parent )
{    
    Q_UNUSED( list )
    QAction* const action = actionCollection()->addAction( "file_onlinesync_sync" );
    action->setText( i18n( "Synchronize Feeds" ) );
    connect( action, SIGNAL( triggered( bool ) ), 
             this, SLOT( doSynchronize() ) );
 
    setXMLFile( "akregator_onlinesync_plugin.rc" , /*merge=*/true );

    // TODO Move all the code that is related to FeedSync
    KActionCollection* coll = actionCollection();
    feedSyncMenu = coll->add<KActionMenu>("feedsync_menu");
    feedSyncMenu->setText(i18n("&Feed synchronization"));
    // connect(d->mainWidget, SIGNAL(feedSyncUpdated()), this, SLOT(loadFeedSyncMenu()));

    // Refresh the menu
    doSynchronize();
}

OnlineSyncPlugin::~OnlineSyncPlugin()
{
}

void OnlineSyncPlugin::doInitialize()
{
}

void OnlineSyncPlugin::doSynchronize()
{
    kDebug();
    // The object that will do the Sync
    feedsync::FeedSync * syncTool = new feedsync::FeedSync();

    // Clear the menubar
    for (int i=0;i<feedSyncAction.count();i++) {
        feedSyncMenu->removeAction(feedSyncAction.at(i));
    }
    feedSyncAction.clear();

    // Fill the menubar
    KActionCollection* coll = actionCollection();
    QAction* action;
    // Read configuration
    KConfig config("akregator_feedsyncrc");
    foreach ( const QString& groupname, config.groupList() ) {
        if (groupname.left(15)=="FeedSyncSource_") {
            kDebug() << groupname;
            KConfigGroup generalGroup( &config, groupname );

            action = coll->addAction(groupname);
            action->setProperty("ConfigGroup",groupname);
            action->setProperty("SyncType",syncTool->Get);
            action->setIcon(KIcon("mail-receive"));
            action->setText(i18n(QString ("Get from "+generalGroup.readEntry( "Identifier", QString() )).toUtf8()));
            feedSyncMenu->addAction(action);
            feedSyncAction.append(action);
            connect( action, SIGNAL(triggered(bool)), syncTool, SLOT(sync()) );

            action = coll->addAction(groupname);
            action->setProperty("ConfigGroup",groupname);
            action->setProperty("SyncType",syncTool->Send);
            action->setIcon(KIcon("mail-send"));
            action->setText(i18n(QString ("Send to "+generalGroup.readEntry( "Identifier", QString() )).toUtf8()));
            feedSyncMenu->addAction(action);
            feedSyncAction.append(action);
            connect( action, SIGNAL(triggered(bool)), syncTool, SLOT(sync()) );
        }
    }

    action = coll->addAction("feedsync_manage");
    action->setIcon(KIcon("application-rss+xml"));
    action->setText(i18n("Manage..."));
    feedSyncMenu->addAction(action);
    feedSyncAction.append(action);
    // connect(action, SIGNAL(triggered(bool)), d->mainWidget, SLOT(slotFeedSyncManage()));
}







// TMP

// void Akregator::MainWidget::slotFeedSyncManage()
// {
//     using namespace feedsync;
//     kDebug();
//     ConfigurationDialog * dlg = new ConfigurationDialog();
//     dlg->show();
//     connect( dlg, SIGNAL( finished() ), this, SLOT( slotFeedSyncManageDone() ) );
// }
// 
// void Akregator::MainWidget::slotFeedSyncManageDone()
// {
//     kDebug();
//     emit feedSyncUpdated();
// }
// 















#include "onlinesyncplugin.moc"
