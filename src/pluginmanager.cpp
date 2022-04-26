/***************************************************************************
    SPDX-FileCopyrightText: 2004 Mark Kretschmann <markey@web.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later                             *
 *                                                                         *
 ***************************************************************************/

#include "pluginmanager.h"
#include "plugin.h"

#include <QString>

#include "akregator_debug.h"
#include <KLocalizedString>
#include <KMessageBox>
#include <KServiceTypeTrader>

using Akregator::Plugin;
using std::vector;

namespace Akregator
{
vector<PluginManager::StoreItem> PluginManager::m_store;

/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE
/////////////////////////////////////////////////////////////////////////////////////

KService::List PluginManager::query(const QString &constraint)
{
    // Add versioning constraint
    QString str = QStringLiteral("[X-KDE-akregator-framework-version] == ");
    str += QString::number(AKREGATOR_PLUGIN_INTERFACE_VERSION);
    str += QStringLiteral(" and ");
    if (!constraint.trimmed().isEmpty()) {
        str += constraint + QStringLiteral(" and ");
    }
    str += QStringLiteral("[X-KDE-akregator-rank] > 0");

    qCDebug(AKREGATOR_LOG) << "Plugin trader constraint:" << str;

    return KServiceTypeTrader::self()->query(QStringLiteral("Akregator/Plugin"), str);
}

Plugin *PluginManager::createFromService(const KService::Ptr &service, QObject *parent)
{
    qCDebug(AKREGATOR_LOG) << "Trying to load:" << service->library();

    KPluginFactory *factory = KPluginFactory::loadFactory(KPluginMetaData(service->library())).plugin;
    if (!factory) {
        qCWarning(AKREGATOR_LOG) << QStringLiteral(" Could not create plugin factory for: %1").arg(service->library());
        return nullptr;
    }
    auto const plugin = factory->create<Plugin>(parent);

    // put plugin into store
    StoreItem item;
    item.plugin = plugin;
    item.service = service;
    m_store.push_back(item);

    dump(service);
    return plugin;
}

void PluginManager::dump(const KService::Ptr &service)
{
    qCDebug(AKREGATOR_LOG) << "PluginManager Service Info:"
                           << "---------------------------"
                           << "name                          : " << service->name() << "library                       : " << service->library()
                           << "desktopEntryPath              : " << service->entryPath()
                           << "X-KDE-akregator-plugintype       : " << service->property(QStringLiteral("X-KDE-akregator-plugintype")).toString()
                           << "X-KDE-akregator-name             : " << service->property(QStringLiteral("X-KDE-akregator-name")).toString()
                           << "X-KDE-akregator-authors          : " << service->property(QStringLiteral("X-KDE-akregator-authors")).toStringList()
                           << "X-KDE-akregator-rank             : " << service->property(QStringLiteral("X-KDE-akregator-rank")).toString()
                           << "X-KDE-akregator-version          : " << service->property(QStringLiteral("X-KDE-akregator-version")).toString()
                           << "X-KDE-akregator-framework-version: " << service->property(QStringLiteral("X-KDE-akregator-framework-version")).toString();
}
} // namespace Akregator
