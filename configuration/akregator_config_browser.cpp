/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akregator_config_browser.h"
#include "akregatorconfig.h"

#include "ui_settings_browser.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorBrowserConfig, "akregator_config_browser.json")

KCMAkregatorBrowserConfig::KCMAkregatorBrowserConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new QWidget(this))
{
    Ui::SettingsBrowser ui;
    ui.setupUi(m_widget);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_widget);

    connect(ui.kcfg_ExternalBrowserUseCustomCommand, &QAbstractButton::toggled, ui.kcfg_ExternalBrowserCustomCommand, &QWidget::setEnabled);
    KAboutData *about = new KAboutData(QStringLiteral("kcmakrbrowserconfig"),
                                       i18n("Configure Feed Reader Browser"),
                                       QString(),
                                       QString(),
                                       KAboutLicense::GPL,
                                       i18n("(c), 2004 - 2008 Frank Osterfeld"));

    about->addAuthor(i18n("Frank Osterfeld"), QString(), QStringLiteral("osterfeld@kde.org"));

    setAboutData(about);
    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_browser.moc"
