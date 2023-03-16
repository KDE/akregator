/*
   SPDX-FileCopyrightText: 2021-2023 Laurent Montel <montel@kde.org>

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
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)

KCMAkregatorSecurityConfig::KCMAkregatorSecurityConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new QWidget(this))
#else
KCMAkregatorSecurityConfig::KCMAkregatorSecurityConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , m_widget(new QWidget(widget()))
#endif
{
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto lay = new QHBoxLayout(this);
#else
    auto lay = new QHBoxLayout(widget());
#endif
    lay->setContentsMargins({});
    Ui::SettingsSecurity ui;
    ui.setupUi(m_widget);
    lay->addWidget(m_widget);

    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_security.moc"
