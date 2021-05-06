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

K_PLUGIN_FACTORY(KCMAkregatorGeneralConfigFactory, registerPlugin<KCMAkregatorGeneralConfig>();)

KCMAkregatorGeneralConfig::KCMAkregatorGeneralConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new QWidget(this))
{
    Ui::SettingsGeneral ui;
    ui.setupUi(m_widget);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_widget);

    ui.kcfg_AutoFetchInterval->setSuffix(ki18np(" minute", " minutes"));

    connect(ui.kcfg_UseIntervalFetch, &QAbstractButton::toggled, ui.kcfg_AutoFetchInterval, &QWidget::setEnabled);
    connect(ui.kcfg_UseIntervalFetch, &QAbstractButton::toggled, ui.autoFetchIntervalLabel, &QWidget::setEnabled);
    auto about = new KAboutData(QStringLiteral("kcmakrgeneralconfig"),
                                i18n("Configure Feeds"),
                                QString(),
                                QString(),
                                KAboutLicense::GPL,
                                i18n("(c), 2004 - 2008 Frank Osterfeld"));

    about->addAuthor(i18n("Frank Osterfeld"), QString(), QStringLiteral("osterfeld@kde.org"));

    setAboutData(about);
    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_general.moc"
