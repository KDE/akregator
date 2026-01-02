/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "akregator_config_advanced.h"
#include "akregatorconfig.h"

#include "settings_advanced.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorAdvancedConfig, "akregator_config_advanced.json")

KCMAkregatorAdvancedConfig::KCMAkregatorAdvancedConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_widget(new SettingsAdvanced(widget()))
{
    auto layout = new QVBoxLayout(widget());
    layout->addWidget(m_widget);
    addConfig(Settings::self(), m_widget);
}

void KCMAkregatorAdvancedConfig::load()
{
    KCModule::load();
}

void KCMAkregatorAdvancedConfig::save()
{
    KCModule::save();
}

#include "akregator_config_advanced.moc"

#include "moc_akregator_config_advanced.cpp"
