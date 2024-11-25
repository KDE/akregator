/*
   SPDX-FileCopyrightText: 2016-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregator_config_plugins.h"
#include "akregatorconfig.h"
#include "akregatorconfigurepluginlistwidget.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <PimCommon/ConfigurePluginsListWidget>
#include <PimCommon/ConfigurePluginsWidget>
#include <QHBoxLayout>
using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorPluginsConfig, "akregator_config_plugins.json")
KCMAkregatorPluginsConfig::KCMAkregatorPluginsConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    auto lay = new QHBoxLayout(widget());
    lay->setContentsMargins({});
    mConfigurePluginWidget = new PimCommon::ConfigurePluginsWidget(new AkregatorConfigurePluginListWidget(widget()), widget());
    mConfigurePluginWidget->setObjectName(QLatin1StringView("configurePluginWidget"));
    connect(mConfigurePluginWidget, &PimCommon::ConfigurePluginsWidget::wasChanged, this, [this](bool state) {
        setNeedsSave(state);
    });
    lay->addWidget(mConfigurePluginWidget);
}

void KCMAkregatorPluginsConfig::slotConfigChanged()
{
    markAsChanged();
}

void KCMAkregatorPluginsConfig::save()
{
    mConfigurePluginWidget->save();
}

void KCMAkregatorPluginsConfig::load()
{
    mConfigurePluginWidget->doLoadFromGlobalSettings();
}

void KCMAkregatorPluginsConfig::defaults()
{
    mConfigurePluginWidget->doResetToDefaultsOther();
}

#include "akregator_config_plugins.moc"

#include "moc_akregator_config_plugins.cpp"
