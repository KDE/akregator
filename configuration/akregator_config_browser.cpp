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

#include "akregator_config_browser.h"
#include "akregatorconfig.h"

#include "ui_settings_browser.h"

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_FACTORY_WITH_JSON(KCMAkregatorBrowserConfigFactory, "akregator_config_browser.json", registerPlugin<KCMAkregatorBrowserConfig>();)

KCMAkregatorBrowserConfig::KCMAkregatorBrowserConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args), m_widget(new QWidget)
{
    Ui::SettingsBrowser ui;
    ui.setupUi(m_widget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_widget);

    connect(ui.kcfg_ExternalBrowserUseCustomCommand, SIGNAL(toggled(bool)),
            ui.kcfg_ExternalBrowserCustomCommand, SLOT(setEnabled(bool)));
    KAboutData *about = new KAboutData(QStringLiteral("kcmakrbrowserconfig"),
                                       i18n("Configure Feed Reader Browser"),
                                       QString(), QString(), KAboutLicense::GPL,
                                       i18n("(c), 2004 - 2008 Frank Osterfeld"));

    about->addAuthor(i18n("Frank Osterfeld"), QString(), QStringLiteral("osterfeld@kde.org"));

    setAboutData(about);
    addConfig(Settings::self(), m_widget);
}
#include "akregator_config_browser.moc"

