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
KCMAkregatorBrowserConfig::KCMAkregatorBrowserConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , m_widget(new QWidget(widget()))
{
    Ui::SettingsBrowser ui;
    ui.setupUi(m_widget);
    auto layout = new QVBoxLayout(widget());
    layout->addWidget(m_widget);

    connect(ui.kcfg_ExternalBrowserUseCustomCommand, &QAbstractButton::toggled, ui.kcfg_ExternalBrowserCustomCommand, &QWidget::setEnabled);
    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_browser.moc"
