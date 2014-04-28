/*
    This file is part of Akregator.

    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>

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

#include "sharemicroblogplugin.h"

#include <KAction>
#include <KActionMenu>
#include <KActionCollection>
#include <KConfigGroup>
#include <KMessageBox>
#include <KXMLGUIClient>
#include <Plasma/DataEngineManager>
#include <Plasma/ServiceJob>


K_PLUGIN_FACTORY(SharePluginFactory, registerPlugin<SharePluginIface>();)
K_EXPORT_PLUGIN(SharePluginFactory("akregator_sharemicroblog_plugin"))

SharePluginIface::SharePluginIface( QObject* parent, const QVariantList& args )
: Plugin( parent, args ), m_impl( new SharePlugin( parent, args ) )
{
}

SharePluginIface::~SharePluginIface()
{
    delete m_impl;
}

void SharePluginIface::doInitialize()
{
    connect(parent(), SIGNAL(signalArticlesSelected(QList<Akregator::Article>)),
            m_impl, SLOT(articlesSelected(QList<Akregator::Article>)));
}

void SharePluginIface::insertGuiClients( KXMLGUIClient* parent )
{
    parent->insertChildClient( m_impl );
}

void SharePluginIface::removeGuiClients( KXMLGUIClient* parent )
{
    parent->removeChildClient( m_impl );
}

SharePlugin::SharePlugin( QObject* parent, const QVariantList& args )
    : KParts::Plugin( parent ), m_shareMenu(0), m_sharePopupMenu(0),
      m_username(QString()), m_service(0)
{
    Q_UNUSED(args);
    setComponentData( SharePluginFactory::componentData() );

    // Share feature provided by Plasma
    m_engine = Plasma::DataEngineManager::self()->loadEngine(QLatin1String("microblog"));
    if (!m_engine->isValid()) {
        qDebug() << "could not load microblog data engine";
        return;
    }

    refreshConfig();

    // configure the ui with the actions
    setXMLFile( QLatin1String("akregator_sharemicroblog_plugin.rc"), /*merge=*/ true );
    KActionCollection* coll = actionCollection();
    m_shareMenu = coll->add<KActionMenu>(QLatin1String("article_share"));
    m_shareMenu->setText( i18n( "Share Article" ) );
    m_shareMenu->setShortcuts(KShortcut(QLatin1String("Ctrl+S")));
    m_shareMenu->setEnabled(false);
    connect(m_shareMenu, SIGNAL(triggered(bool)), this, SLOT(shareArticles()));

    m_sharePopupMenu = coll->add<KActionMenu>(QLatin1String("article_share_popup"));
    m_sharePopupMenu->setText( i18n( "Share Article" ) );
    m_sharePopupMenu->setEnabled(false);
    connect(m_sharePopupMenu, SIGNAL(triggered(bool)), this, SLOT(shareArticles()));
}

SharePlugin::~SharePlugin()
{
    if (m_service) {
        delete m_service;
        Plasma::DataEngineManager::self()->unloadEngine(QLatin1String("microblog"));
    }
}

void SharePlugin::refreshConfig()
{
    const QString timeline = QLatin1String("TimelineWithFriends:%1@%2");

    // remove current stuff
    delete m_service;
    m_engine->disconnectSource(timeline.arg(m_username, m_serviceUrl), this);

    // read config files and refresh
    const KConfig config(QLatin1String("akregator_sharerc"));
    const KConfigGroup group(&config, "ShareService");

    m_username = group.readEntry("Username", "");
    m_serviceUrl = group.readEntry("ServiceUrl", "https://identi.ca/api/");

    if (!m_username.isEmpty()) {
        const QString m_curTimeline(timeline.arg(m_username, m_serviceUrl));
        m_engine->connectSource(m_curTimeline, this);
        m_service = m_engine->serviceForSource(m_curTimeline);
        connect(m_service, SIGNAL(finished(Plasma::ServiceJob*)),
                this, SLOT(slotServiceFinished(Plasma::ServiceJob*)));
    }
}

void SharePlugin::articlesSelected(const QList<Akregator::Article> &articles)
{
    m_articles = articles;
    if (m_shareMenu) m_shareMenu->setEnabled(true);
    if (m_sharePopupMenu) m_sharePopupMenu->setEnabled(true);
}

void SharePlugin::shareArticles()
{
    // give a second chance as the user may have configured the service
    if (m_username.isEmpty()) {
        refreshConfig();
    }

    // it doesn't have a valid configuration. warn the user.
    if (!m_service) {
        KMessageBox::sorry(0, i18n("Please, configure the share service before using it."),
                           i18n("Service not configured"));
        return;
    }

    // setup the service and create the status message
    KConfigGroup cg = m_service->operationDescription(QLatin1String("update"));
    foreach(const Akregator::Article& article, m_articles) {
        QString status = QString::fromLatin1("%1 - %2 #share").arg(article.title(),
                                                       article.link().prettyUrl());
        cg.writeEntry("status", status);
        m_service->startOperationCall(cg);
    }
}

void SharePlugin::slotServiceFinished(Plasma::ServiceJob *job)
{
    if (job->error()) {
        KMessageBox::error(0, i18n("Sorry, could not share the article: %1", job->errorText()),
                           i18n("Error during article share"));
    }
}

void SharePlugin::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    // not used right now
    // however we must have this method to properly use the dataengine
    Q_UNUSED(source);
    Q_UNUSED(data);
}


