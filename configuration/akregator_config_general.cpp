/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akregator_config_general.h"
#include "akregatorconfig.h"
#include "config-akregator.h"

#include "ui_settings_general.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

#include <KLocalization>
#include <QVBoxLayout>
#include <ki18n_version.h>
using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorGeneralConfig, "akregator_config_general.json")
KCMAkregatorGeneralConfig::KCMAkregatorGeneralConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_widget(new QWidget(widget()))
{
    Ui::SettingsGeneral ui;
    ui.setupUi(m_widget);

    auto layout = new QVBoxLayout(widget());
    layout->addWidget(m_widget);

#if KI18N_VERSION > QT_VERSION_CHECK(6, 5, 0)
    KLocalization::setupSpinBoxFormatString(ui.kcfg_AutoFetchInterval, ki18np(" minute", " minutes"));
#endif

    connect(ui.kcfg_UseIntervalFetch, &QAbstractButton::toggled, ui.kcfg_AutoFetchInterval, &QWidget::setEnabled);
    connect(ui.kcfg_UseIntervalFetch, &QAbstractButton::toggled, ui.autoFetchIntervalLabel, &QWidget::setEnabled);
    addConfig(Settings::self(), m_widget);
#if !HAVE_ACTIVITY_SUPPORT
    ui.kcfg_PlasmaActivitySupport->hide();
#endif
}

#include "akregator_config_general.moc"

#include "moc_akregator_config_general.cpp"
