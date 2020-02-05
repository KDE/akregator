/*
    This file is part of Akregator.
    Copyright (c) 2008 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
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
    QVBoxLayout *layout = new QVBoxLayout(this);
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
