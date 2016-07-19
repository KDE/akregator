/***************************************************************************
begin                : 2004/03/12
copyright            : (C) Mark Kretschmann
email                : markey@web.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pluginmanager.h"
#include "plugin.h"

#include <vector>
#include <QString>

#include "akregator_debug.h"
#include <KLocalizedString>
#include <kmessagebox.h>

using std::vector;
using Akregator::Plugin;

namespace Akregator
{

vector<PluginManager::StoreItem>
PluginManager::m_store;

/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE
/////////////////////////////////////////////////////////////////////////////////////

KService::List
PluginManager::query(const QString &constraint)
{
    // Add versioning constraint
    QString
    str  = QStringLiteral("[X-KDE-akregator-framework-version] == ");
    str += QString::number(AKREGATOR_PLUGIN_INTERFACE_VERSION);
    str += QStringLiteral(" and ");
    if (!constraint.trimmed().isEmpty()) {
        str += constraint + QStringLiteral(" and ");
    }
    str += QStringLiteral("[X-KDE-akregator-rank] > 0");

    qCDebug(AKREGATOR_LOG) << "Plugin trader constraint:" << str;

    return KServiceTypeTrader::self()->query(QStringLiteral("Akregator/Plugin"), str);
}

Plugin *
PluginManager::createFromQuery(const QString &constraint)
{
    KService::List offers = query(constraint);

    if (offers.isEmpty()) {
        qCWarning(AKREGATOR_LOG) << "No matching plugin found.";
        return 0;
    }

    // Select plugin with highest rank
    int rank = 0;
    uint current = 0;
    for (int i = 0; i < offers.count(); ++i) {
        if (offers[i]->property(QStringLiteral("X-KDE-akregator-rank")).toInt() > rank) {
            current = i;
        }
    }

    return createFromService(offers[current]);
}

Plugin *
PluginManager::createFromService(const KService::Ptr &service, QObject *parent)
{
    qCDebug(AKREGATOR_LOG) << "Trying to load:" << service->library();

    KPluginLoader loader(*service);
    KPluginFactory *factory = loader.factory();
    if (!factory) {
        qCWarning(AKREGATOR_LOG) << QStringLiteral(" Could not create plugin factory for: %1\n"
                                 " Error message: %2").arg(service->library(), loader.errorString());
        return 0;
    }
    Plugin *const plugin = factory->create<Plugin>(parent);

    //put plugin into store
    StoreItem item;
    item.plugin = plugin;
    item.service = service;
    m_store.push_back(item);

    dump(service);
    return plugin;
}

void
PluginManager::unload(Plugin *plugin)
{
#ifdef TEMPORARILY_REMOVED
    vector<StoreItem>::iterator iter = lookupPlugin(plugin);

    if (iter != m_store.end()) {
        delete(*iter).plugin;
        qCDebug(AKREGATOR_LOG) << "Unloading library:" << (*iter).service->library();
        //PENDING(kdab,frank) Review
        (*iter).library->unload();

        m_store.erase(iter);
    } else {
        qCWarning(AKREGATOR_LOG) << "Could not unload plugin (not found in store).";
    }
#else //TEMPORARILY_REMOVED
    Q_UNUSED(plugin)
    qCWarning(AKREGATOR_LOG) << "PluginManager::unload temporarily disabled";
#endif //TEMPORARILY_REMOVED

}

KService::Ptr
PluginManager::getService(const Plugin *plugin)
{
    if (!plugin) {
        qCWarning(AKREGATOR_LOG) << "pointer == NULL";
        return KService::Ptr(0);
    }

    //search plugin in store
    vector<StoreItem>::const_iterator iter = lookupPlugin(plugin);

    if (iter == m_store.end()) {
        qCWarning(AKREGATOR_LOG) << "Plugin not found in store.";
        return KService::Ptr(0);
    }

    return (*iter).service;
}

void
PluginManager::showAbout(const QString &constraint)
{
    KService::List offers = query(constraint);

    if (offers.isEmpty()) {
        return;
    }

    KService::Ptr s = offers.front();

    const QString body = QStringLiteral("<tr><td>%1</td><td>%2</td></tr>");

    QString str  = QStringLiteral("<html><body><table width=\"100%\" border=\"1\">");

    str += body.arg(i18nc("Name of the plugin", "Name"),                             s->name());
    str += body.arg(i18nc("Library name", "Library"),                                s->library());
    str += body.arg(i18nc("Plugin authors", "Authors"),                              s->property(QStringLiteral("X-KDE-akregator-authors")).toStringList().join(QLatin1Char('\n')));
    str += body.arg(i18nc("Plugin authors' emaila addresses", "Email"),              s->property(QStringLiteral("X-KDE-akregator-email")).toStringList().join(QLatin1Char('\n')));
    str += body.arg(i18nc("Plugin version", "Version"),                              s->property(QStringLiteral("X-KDE-akregator-version")).toString());
    str += body.arg(i18nc("Framework version plugin requires", "Framework Version"), s->property(QStringLiteral("X-KDE-akregator-framework-version")).toString());

    str += QStringLiteral("</table></body></html>");

    KMessageBox::information(0, str, i18n("Plugin Information"));
}

void
PluginManager::dump(const KService::Ptr &service)
{
    qCDebug(AKREGATOR_LOG)
            << "PluginManager Service Info:"
            << "---------------------------"
            << "name                          : " << service->name()
            << "library                       : " << service->library()
            << "desktopEntryPath              : " << service->entryPath()
            << "X-KDE-akregator-plugintype       : " << service->property(QStringLiteral("X-KDE-akregator-plugintype")).toString()
            << "X-KDE-akregator-name             : " << service->property(QStringLiteral("X-KDE-akregator-name")).toString()
            << "X-KDE-akregator-authors          : " << service->property(QStringLiteral("X-KDE-akregator-authors")).toStringList()
            << "X-KDE-akregator-rank             : " << service->property(QStringLiteral("X-KDE-akregator-rank")).toString()
            << "X-KDE-akregator-version          : " << service->property(QStringLiteral("X-KDE-akregator-version")).toString()
            << "X-KDE-akregator-framework-version: " << service->property(QStringLiteral("X-KDE-akregator-framework-version")).toString();

}

/////////////////////////////////////////////////////////////////////////////////////
// PRIVATE INTERFACE
/////////////////////////////////////////////////////////////////////////////////////

vector<PluginManager::StoreItem>::iterator
PluginManager::lookupPlugin(const Plugin *plugin)
{
    vector<StoreItem>::iterator iter;

    //search plugin pointer in store
    vector<StoreItem>::const_iterator end;
    for (iter = m_store.begin(); iter != end; ++iter) {
        if ((*iter).plugin == plugin) {
            break;
        }
    }

    return iter;
}

} // namespace Akregator
