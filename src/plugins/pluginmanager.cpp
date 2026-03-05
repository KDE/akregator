/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "pluginmanager.h"
#include "accountplugin.h"

#include <KPluginFactory>
#include <KPluginMetaData>

#include <QLoggingCategory>

static const QLoggingCategory AKREGATOR_PLUGIN_LOG("org.kde.pim.akregator");

using namespace Akregator;

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
}

PluginManager::~PluginManager() = default;

void PluginManager::instantiatePlugins()
{
    const auto plugins = KPluginMetaData::findPlugins(QStringLiteral("pim6/akregator/accountplugins"));
    for (const auto &meta : plugins) {
        auto result = KPluginFactory::instantiatePlugin<AccountPlugin>(meta, this);
        if (result) {
            m_accountPlugins.append(result.plugin);
            qCDebug(AKREGATOR_PLUGIN_LOG) << "Instantiated account plugin:" << result.plugin->name();
        } else {
            qCWarning(AKREGATOR_PLUGIN_LOG) << "Failed to load account plugin:" << meta.fileName() << result.errorString;
        }
    }
}

void PluginManager::initializePlugins(FeedList *feedList)
{
    for (AccountPlugin *plugin : std::as_const(m_accountPlugins)) {
        plugin->initialize(feedList);
    }
}

QList<AccountPlugin *> PluginManager::accountPlugins() const
{
    return m_accountPlugins;
}
