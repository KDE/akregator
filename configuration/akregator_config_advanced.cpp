/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akregator_config_advanced.h"
#include "akregatorconfig.h"

#include "settings_advanced.h"

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorAdvancedConfig, "akregator_config_advanced.json")

KCMAkregatorAdvancedConfig::KCMAkregatorAdvancedConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new SettingsAdvanced(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_widget);
    KAboutData *about = new KAboutData(QStringLiteral("kcmakradvancedconfig"),
                                       i18n("Advanced Feed Reader Settings"),
                                       QString(), QString(), KAboutLicense::GPL,
                                       i18n("(c), 2004 - 2008 Frank Osterfeld"));

    about->addAuthor(i18n("Frank Osterfeld"), QString(), QStringLiteral("osterfeld@kde.org"));

    setAboutData(about);
    addConfig(Settings::self(), m_widget);
}

void KCMAkregatorAdvancedConfig::load()
{
    KCModule::load();
    m_widget->selectFactory(Settings::archiveBackend());
}

void KCMAkregatorAdvancedConfig::save()
{
    Settings::setArchiveBackend(m_widget->selectedFactory());
    KCModule::save();
}

#include "akregator_config_advanced.moc"
