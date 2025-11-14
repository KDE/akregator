/*
   SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregatorconfigurepluginlistwidget.h"
#include "kcm_config_plugins_debug.h"
#include <KLocalizedString>
#include <PimCommon/PluginUtil>
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
    : TextAddonsWidgets::ConfigurePluginsWidget(parent)
{
    connect(this, &TextAddonsWidgets::ConfigurePluginsWidget::configureClicked, this, &AkregatorConfigurePluginListWidget::slotConfigureClicked);
}

AkregatorConfigurePluginListWidget::~AkregatorConfigurePluginListWidget() = default;

void AkregatorConfigurePluginListWidget::save()
{
    TextAddonsWidgets::ConfigurePluginsWidget::savePlugins(WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configGroupName(),
                                                           WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configPrefixSettingKey(),
                                                           mPluginWebEngineItems,
                                                           PimCommon::PluginUtil::pluginConfigFile());
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
    mTreePluginWidget->clear();
    // Load webengineplugin
    TextAddonsWidgets::ConfigurePluginsWidget::fillTopItems(WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->pluginsDataList(),
                                                            i18n("Webengine Plugins"),
                                                            WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configGroupName(),
                                                            WebEngineViewer::NetworkUrlInterceptorPluginManager::self()->configPrefixSettingKey(),
                                                            mPluginWebEngineItems,
                                                            networkUrlInterceptorGroupName(),
                                                            true,
                                                            PimCommon::PluginUtil::pluginConfigFile());
    mTreePluginWidget->expandAll();
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
