/*
   SPDX-FileCopyrightText: 2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregator_config_security.h"
#include "akregatorconfig.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QHBoxLayout>
using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorSecurityConfigFactory, registerPlugin<KCMAkregatorSecurityConfig>();)

KCMAkregatorSecurityConfig::KCMAkregatorSecurityConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    auto lay = new QHBoxLayout(this);
    lay->setContentsMargins({});

    auto about = new KAboutData(QStringLiteral("kcmakrsecutiryconfig"),
                                i18n("Configure Plugins"),
                                QString(),
                                QString(),
                                KAboutLicense::GPL,
                                i18n("(c), 2021 Laurent Montel"));

    about->addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));

    setAboutData(about);
    //    mConfigurePluginWidget = new PimCommon::ConfigurePluginsWidget(new AkregatorConfigurePluginListWidget(this), this);
    //    mConfigurePluginWidget->setObjectName(QStringLiteral("configurePluginWidget"));
    //    connect(mConfigurePluginWidget, &PimCommon::ConfigurePluginsWidget::changed, this, &KCMAkregatorSecurityConfig::slotConfigChanged);
    //    lay->addWidget(mConfigurePluginWidget);
}

void KCMAkregatorSecurityConfig::slotConfigChanged()
{
    Q_EMIT changed(true);
}

void KCMAkregatorSecurityConfig::save()
{
    //    mConfigurePluginWidget->save();
}

void KCMAkregatorSecurityConfig::load()
{
    //    mConfigurePluginWidget->doLoadFromGlobalSettings();
}

void KCMAkregatorSecurityConfig::defaults()
{
    //    mConfigurePluginWidget->doResetToDefaultsOther();
}

#include "akregator_config_security.moc"
