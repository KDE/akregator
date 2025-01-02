/*
   SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregatorconfigurepluginlistwidget.h"
#include "kcm_config_plugins_debug.h"
#include <KLocalizedString>
#include <WebEngineViewer/NetworkPluginUrlInterceptor>
#include <WebEngineViewer/NetworkUrlInterceptorPluginManager>
namespace
{
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

AkregatorConfigurePluginListWidget::~AkregatorConfigurePluginListWidget() = default;

void AkregatorConfigurePluginListWidget::save()
{
    PimCommon::ConfigurePluginsListWidget::savePlugins(WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configGroupName(),
                                                       WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configPrefixSettingKey(),
                                                       mPluginWebEngineItems);
}

void AkregatorConfigurePluginListWidget::doLoadFromGlobalSettings()
{
    initialize();
    initializeDone();
}

void AkregatorConfigurePluginListWidget::doResetToDefaultsOther()
{
    changeState(mPluginWebEngineItems);
}

void AkregatorConfigurePluginListWidget::initialize()
{
    mListWidget->clear();
    // Load webengineplugin
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
            WebEngineViewer::NetworkPluginUrlInterceptor *plugin =
                WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->pluginFromIdentifier(identifier);
            plugin->showConfigureDialog(this);
        } else {
            qCWarning(AKREGATOR_CONFIG_PLUGIN_LOG) << "Unknown configureGroupName" << configureGroupName;
        }
    }
}

#include "moc_akregatorconfigurepluginlistwidget.cpp"
