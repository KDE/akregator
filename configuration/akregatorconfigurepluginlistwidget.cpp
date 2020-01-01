/*
   Copyright (C) 2016-2020 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "akregatorconfigurepluginlistwidget.h"
#include <WebEngineViewer/NetworkUrlInterceptorPluginManager>
#include <WebEngineViewer/NetworkPluginUrlInterceptor>
#include "kcm_config_plugins_debug.h"
#include <KLocalizedString>
namespace {
QString networkUrlInterceptorGroupName()
{
    return QStringLiteral("networkurlinterceptorgroupname");
}
}

AkregatorConfigurePluginListWidget::AkregatorConfigurePluginListWidget(QWidget *parent)
    : PimCommon::ConfigurePluginsListWidget(parent)
{
    connect(this, &ConfigurePluginsListWidget::configureClicked, this, &AkregatorConfigurePluginListWidget::slotConfigureClicked);
}

AkregatorConfigurePluginListWidget::~AkregatorConfigurePluginListWidget()
{
}

void AkregatorConfigurePluginListWidget::save()
{
    PimCommon::ConfigurePluginsListWidget::savePlugins(WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configGroupName(),
                                                       WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configPrefixSettingKey(),
                                                       mPluginWebEngineItems);
}

void AkregatorConfigurePluginListWidget::doLoadFromGlobalSettings()
{
    initialize();
}

void AkregatorConfigurePluginListWidget::doResetToDefaultsOther()
{
    changeState(mPluginWebEngineItems);
}

void AkregatorConfigurePluginListWidget::initialize()
{
    mListWidget->clear();
    //Load webengineplugin
    PimCommon::ConfigurePluginsListWidget::fillTopItems(WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->pluginsDataList(),
                                                        i18n("Webengine Plugins"),
                                                        WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configGroupName(),
                                                        WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configPrefixSettingKey(),
                                                        mPluginWebEngineItems,
                                                        networkUrlInterceptorGroupName());
    mListWidget->expandAll();
}

void AkregatorConfigurePluginListWidget::slotConfigureClicked(const QString &configureGroupName, const QString &identifier)
{
    if (!configureGroupName.isEmpty() && !identifier.isEmpty()) {
        if (configureGroupName == networkUrlInterceptorGroupName()) {
            WebEngineViewer::NetworkPluginUrlInterceptor *plugin = WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->pluginFromIdentifier(identifier);
            plugin->showConfigureDialog(this);
        } else {
            qCWarning(AKREGATOR_CONFIG_PLUGIN_LOG) << "Unknown configureGroupName" << configureGroupName;
        }
    }
}
