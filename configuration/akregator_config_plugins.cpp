/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregator_config_plugins.h"
#include "akregatorconfig.h"
#include "akregatorconfigurepluginlistwidget.h"
#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>
#include <PimCommon/ConfigurePluginsWidget>
#include <PimCommon/ConfigurePluginsListWidget>
#include <QHBoxLayout>
using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorPluginsConfigFactory, registerPlugin<KCMAkregatorPluginsConfig>();
                 )

KCMAkregatorPluginsConfig::KCMAkregatorPluginsConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    KAboutData *about = new KAboutData(QStringLiteral("kcmakrpluginsconfig"),
                                       i18n("Configure Plugins"),
                                       QString(), QString(), KAboutLicense::GPL,
                                       i18n("(c), 2016-2020 Laurent Montel"));

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
