/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akregator_config_general.h"
#include "akregatorconfig.h"

#include "ui_settings_general.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorGeneralConfig, "akregator_config_general.json")
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
KCMAkregatorGeneralConfig::KCMAkregatorGeneralConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new QWidget(this))
#else
KCMAkregatorGeneralConfig::KCMAkregatorGeneralConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , m_widget(new QWidget(widget()))
#endif
{
    Ui::SettingsGeneral ui;
    ui.setupUi(m_widget);

#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto layout = new QVBoxLayout(this);
#else
    auto layout = new QVBoxLayout(widget());
#endif
    layout->addWidget(m_widget);

    ui.kcfg_AutoFetchInterval->setSuffix(ki18np(" minute", " minutes"));

    connect(ui.kcfg_UseIntervalFetch, &QAbstractButton::toggled, ui.kcfg_AutoFetchInterval, &QWidget::setEnabled);
    connect(ui.kcfg_UseIntervalFetch, &QAbstractButton::toggled, ui.autoFetchIntervalLabel, &QWidget::setEnabled);
    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_general.moc"
