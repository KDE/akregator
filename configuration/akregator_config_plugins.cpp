/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

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

KCMAkregatorPluginsConfig::KCMAkregatorPluginsConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    auto lay = new QHBoxLayout(this);
    lay->setContentsMargins({});

    auto about = new KAboutData(QStringLiteral("kcmakrpluginsconfig"),
                                i18n("Configure Plugins"),
                                QString(),
                                QString(),
                                KAboutLicense::GPL,
                                i18n("(c), 2016-2021 Laurent Montel"));

    about->addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));

    setAboutData(about);
    mConfigurePluginWidget = new PimCommon::ConfigurePluginsWidget(new AkregatorConfigurePluginListWidget(this), this);
    mConfigurePluginWidget->setObjectName(QStringLiteral("configurePluginWidget"));
    connect(mConfigurePluginWidget, &PimCommon::ConfigurePluginsWidget::changed, this, &KCMAkregatorPluginsConfig::slotConfigChanged);
    lay->addWidget(mConfigurePluginWidget);
}

void KCMAkregatorPluginsConfig::slotConfigChanged()
{
    Q_EMIT changed(true);
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
