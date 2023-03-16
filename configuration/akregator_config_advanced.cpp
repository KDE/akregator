/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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

#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
KCMAkregatorAdvancedConfig::KCMAkregatorAdvancedConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new SettingsAdvanced(this))
#else
KCMAkregatorAdvancedConfig::KCMAkregatorAdvancedConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
    , m_widget(new SettingsAdvanced(widget()))
#endif
{
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto layout = new QVBoxLayout(this);
#else
    auto layout = new QVBoxLayout(widget());
#endif
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
