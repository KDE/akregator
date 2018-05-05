/*
 * This file is part of akregatorstorageexporter
 *
 * Copyright (C) 2018 Pierre Ducroquet <pinaraf@pinaraf.info>
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
#include "feedstorage.h"
#include "storage.h"
#include "storagefactory.h"
#include "storagefactoryregistry.h"
#include "plugin.h"


#include <KPluginLoader>
#include <KService>
#include <KServiceTypeTrader>

#include <QCommandLineParser>
#include <QDateTime>
#include <QDebug>


using namespace Akregator;
using namespace Akregator::Backend;

static KService::List queryStoragePlugins()
{
    const QString query = QStringLiteral("[X-KDE-akregator-framework-version] == %1 and [X-KDE-akregator-plugintype] == 'storage' and [X-KDE-akregator-rank] > 0")
                                    .arg(QString::number(AKREGATOR_PLUGIN_INTERFACE_VERSION));

    return KServiceTypeTrader::self()->query(QStringLiteral("Akregator/Plugin"),
                                             query);
}

static Plugin *createFromService(const KService::Ptr &service)
{
    KPluginLoader loader(*service);
    KPluginFactory *factory = loader.factory();
    if (!factory) {
        qCritical() << QStringLiteral(" Could not create plugin factory for: %1\n"
                                      " Error message: %2").arg(service->library(), loader.errorString());
        return nullptr;
    }
    return factory->create<Akregator::Plugin>();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Akregator storage migrator"));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("source"), QStringLiteral("Source backend"));
    parser.addPositionalArgument(QStringLiteral("destination"), QStringLiteral("Destination backend"));
    
    parser.process(app);
    
    if (parser.positionalArguments().size() != 2) {
        parser.showHelp();
        return 1;
    }
    
    // Initialize plugins
    for (const KService::Ptr &i: queryStoragePlugins()) {
        if (Plugin *const plugin = createFromService(i)) {
            plugin->initialize();
        }
    }
    
    const StorageFactory *const sourceStorageFactory = StorageFactoryRegistry::self()->getFactory(parser.positionalArguments()[0]);
    if (!sourceStorageFactory) {
        qCritical() << QStringLiteral("Could not get the source storage factory");
        return 1;
    }
    
    Storage *sourceStorage = sourceStorageFactory->createStorage(QStringList());
    if (!sourceStorage) {
        qCritical() << QStringLiteral("Could not create the source storage");
        return 2;
    }
    
    if (!sourceStorage->open(true)) {
        qCritical() << QStringLiteral("Could not open the source storage");
        return 3;
    }
    
    const StorageFactory *const targetStorageFactory = StorageFactoryRegistry::self()->getFactory(parser.positionalArguments()[1]);
    if (!targetStorageFactory) {
        qCritical() << QStringLiteral("Could not get the target storage factory");
        return 4;
    }
    
    Storage *targetStorage = targetStorageFactory->createStorage(QStringList());
    if (!targetStorage) {
        qCritical() << QStringLiteral("Could not create the target storage");
        return 5;
    }
    
    if (!targetStorage->open(false)) {
        qCritical() << QStringLiteral("Could not open the target storage");
        return 6;
    }
    
    
    qDebug() << "Migrating feedList";
    targetStorage->storeFeedList(sourceStorage->restoreFeedList());
    
    qDebug() << "Migrating feeds...";
    for (const QString &feed: sourceStorage->feeds()) {
        qDebug() << "Migrating " << feed;
        auto sourceFeed = sourceStorage->archiveFor(feed);
        auto targetFeed = targetStorage->archiveFor(feed);
        auto targetExistingArticles = targetFeed->articles();
        
        for (auto &guid: sourceFeed->articles()) {
            //qDebug() << guid;
            if (targetExistingArticles.contains(guid))
                continue;
            
            targetFeed->addEntry(guid);
            
            targetFeed->setStatus(guid, sourceFeed->status(guid));
            targetFeed->setHash(guid, sourceFeed->hash(guid));
            targetFeed->setTitle(guid, sourceFeed->title(guid));
            targetFeed->setContent(guid, sourceFeed->content(guid));
            targetFeed->setDescription(guid, sourceFeed->description(guid));
            targetFeed->setLink(guid, sourceFeed->link(guid));
            targetFeed->setGuidIsPermaLink(guid, sourceFeed->guidIsPermaLink(guid));
            targetFeed->setGuidIsHash(guid, sourceFeed->guidIsHash(guid));
            targetFeed->setPubDate(guid, sourceFeed->pubDate(guid));
            targetFeed->setAuthorName(guid, sourceFeed->authorName(guid));
            targetFeed->setAuthorUri(guid, sourceFeed->authorUri(guid));
            targetFeed->setAuthorEMail(guid, sourceFeed->authorEMail(guid));
            bool hasEnclosure;
            QString enclosureUrl, enclosureType;
            int enclosureLength;
            sourceFeed->enclosure(guid, hasEnclosure, enclosureUrl, enclosureType, enclosureLength);
            if (hasEnclosure)
                targetFeed->setEnclosure(guid, enclosureUrl, enclosureType, enclosureLength);
        }
        
        targetStorage->commit();
    }
    qDebug() << "Migration done !";

    return 0;
}

