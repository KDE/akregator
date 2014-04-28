/*
    This file is part of Akregator.

    Copyright (C) 2008 Didier Hoarau <did.hoarau@gmail.com>

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

#include "onlinesyncplugin.h"
#include "ui/configurationdialog.h"
#include "sync/feedsync.h"

#include <KAction>
#include <KActionMenu>
#include <KActionCollection>
#include <KLocalizedString>
#include <KConfigGroup>

using namespace Akregator;
using namespace feedsync;

K_PLUGIN_FACTORY(OnlineSyncPluginFactory,
                 registerPlugin<Akregator::OnlineSyncPluginIface>();
)
K_EXPORT_PLUGIN(OnlineSyncPluginFactory( "akregator_onlinesync_plugin" ) )


OnlineSyncPluginIface::OnlineSyncPluginIface( QObject* parent, const QList<QVariant>& args ) : Plugin( parent ), m_impl( new OnlineSyncPlugin( parent, args ) )
{
}

OnlineSyncPluginIface::~OnlineSyncPluginIface() {
    delete m_impl;
}

OnlineSyncPlugin::OnlineSyncPlugin( QObject* parent, const QList<QVariant>& args ) : KParts::Plugin( parent ), m_syncTool( new FeedSync( this ) )
{ 
    Q_UNUSED( args )
    setComponentData( OnlineSyncPluginFactory::componentData() );

    setXMLFile( "akregator_onlinesync_plugin.rc" , /*merge=*/true );
    KActionCollection* coll = actionCollection();
    m_feedSyncMenu = coll->add<KActionMenu>("file_onlinesync_sync");
    m_feedSyncMenu->setText(i18n("Synchronize Feeds"));

    // Fill
    updateActions();
}

OnlineSyncPlugin::~OnlineSyncPlugin()
{
    qDebug();
}

void OnlineSyncPlugin::updateActions()
{
    qDebug();

    // Clear the menubar
    Q_FOREACH( KAction* const i, m_feedSyncActions )
        m_feedSyncMenu->removeAction( i );
    qDeleteAll( m_feedSyncActions );
    m_feedSyncActions.clear();

    // Fill the menubar
    KActionCollection* coll = actionCollection();
    KAction* action;
    // Read configuration
    const KConfig config("akregator_feedsyncrc");
    Q_FOREACH ( const QString& groupname, config.groupList() ) {
        if ( groupname.startsWith( QLatin1String("FeedSyncSource_") ) ) {
            qDebug() << groupname;
            KConfigGroup generalGroup( &config, groupname );

            action = coll->addAction(groupname);
            action->setProperty("ConfigGroup",groupname);
            action->setProperty("SyncType", m_syncTool->Get);
            action->setIcon(KIcon("mail-receive"));
            action->setText(i18n("Get from %1",generalGroup.readEntry( "Identifier", QString() )));
            m_feedSyncMenu->addAction(action);
            m_feedSyncActions.append(action);
            connect( action, SIGNAL(triggered(bool)), m_syncTool, SLOT(sync()) );

            action = coll->addAction(groupname);
            action->setProperty("ConfigGroup",groupname);
            action->setProperty("SyncType", m_syncTool->Send);
            action->setIcon(KIcon("mail-send"));
            action->setText(i18n("Send to %1",generalGroup.readEntry( "Identifier", QString() )));
            m_feedSyncMenu->addAction(action);
            m_feedSyncActions.append(action);
            connect( action, SIGNAL(triggered(bool)), m_syncTool, SLOT(sync()) );
        }
    }

    action = coll->addAction("feedsync_manage");
    action->setIcon(KIcon("application-rss+xml"));
    action->setText(i18n("Manage..."));
    m_feedSyncMenu->addAction(action);
    m_feedSyncActions.append(action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotFeedSyncManage()));
}

void OnlineSyncPlugin::doSynchronize()
{
    qDebug();
}

void OnlineSyncPlugin::slotFeedSyncManage()
{
    qDebug();
    feedsync::ConfigurationDialog* dlg = new feedsync::ConfigurationDialog();
    dlg->setAttribute( Qt::WA_DeleteOnClose );
    connect( dlg, SIGNAL(finished()), this, SLOT(slotFeedSyncManageDone()) );
    dlg->show();
}

void OnlineSyncPlugin::slotFeedSyncManageDone()
{
    qDebug();
    updateActions();
}

void OnlineSyncPluginIface::insertGuiClients( KXMLGUIClient* parent ) {
    parent->insertChildClient( m_impl );
}

void OnlineSyncPluginIface::removeGuiClients( KXMLGUIClient* parent ) {
    parent->removeChildClient( m_impl );
}

