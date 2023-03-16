/*
   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>

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
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)

KCMAkregatorPluginsConfig::KCMAkregatorPluginsConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
#else
KCMAkregatorPluginsConfig::KCMAkregatorPluginsConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)

#endif
{
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto lay = new QHBoxLayout(this);
#else
    auto lay = new QHBoxLayout(widget());
#endif
    lay->setContentsMargins({});
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    mConfigurePluginWidget = new PimCommon::ConfigurePluginsWidget(new AkregatorConfigurePluginListWidget(this), this);
#else
    mConfigurePluginWidget = new PimCommon::ConfigurePluginsWidget(new AkregatorConfigurePluginListWidget(widget()), widget());
#endif
    mConfigurePluginWidget->setObjectName(QStringLiteral("configurePluginWidget"));
    connect(mConfigurePluginWidget, &PimCommon::ConfigurePluginsWidget::changed, this, &KCMAkregatorPluginsConfig::slotConfigChanged);
    lay->addWidget(mConfigurePluginWidget);
}

void KCMAkregatorPluginsConfig::slotConfigChanged()
{
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    Q_EMIT changed(true);
#else
    markAsChanged();
#endif
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
