/*
   SPDX-FileCopyrightText: 2021-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregator_config_security.h"
#include "akregatorconfig.h"
#include "ui_settings_security.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QHBoxLayout>
using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorSecurityConfig, "akregator_config_security.json")

KCMAkregatorSecurityConfig::KCMAkregatorSecurityConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new QWidget(this))
{
    auto lay = new QHBoxLayout(this);
    lay->setContentsMargins({});
    Ui::SettingsSecurity ui;
    ui.setupUi(m_widget);
    lay->addWidget(m_widget);

    auto about = new KAboutData(QStringLiteral("kcmakrsecutiryconfig"),
                                i18n("Configure Plugins"),
                                QString(),
                                QString(),
                                KAboutLicense::GPL,
                                i18n("(c), 2021 Laurent Montel"));

    about->addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));

    setAboutData(about);
    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_security.moc"
