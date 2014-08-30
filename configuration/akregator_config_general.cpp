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

#include "akregator_config_general.h"
#include "akregatorconfig.h"

#include "ui_settings_general.h"

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorGeneralConfigFactory, registerPlugin<KCMAkregatorGeneralConfig>();)

KCMAkregatorGeneralConfig::KCMAkregatorGeneralConfig( QWidget* parent, const QVariantList& args )
    : KCModule( parent, args ), m_widget( new QWidget )
{  
    Ui::SettingsGeneral ui;
    ui.setupUi( m_widget );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_widget );

    ui.kcfg_AutoFetchInterval->setSuffix(ki18np(" minute", " minutes"));
    
    connect( ui.kcfg_UseIntervalFetch, SIGNAL(toggled(bool)),
             ui.kcfg_AutoFetchInterval, SLOT(setEnabled(bool)) );
    connect( ui.kcfg_UseIntervalFetch, SIGNAL(toggled(bool)),
             ui.autoFetchIntervalLabel, SLOT(setEnabled(bool)) );
    KAboutData *about = new KAboutData( QLatin1String( "kcmakrgeneralconfig" ),
                                        i18n( "Configure Feeds" ),
                                        QString(), QString(), KAboutLicense::GPL,
                                        i18n( "(c), 2004 - 2008 Frank Osterfeld" ) );

    about->addAuthor( i18n( "Frank Osterfeld" ), QString(), QStringLiteral("osterfeld@kde.org") );

    setAboutData( about );
    addConfig( Settings::self(), m_widget );
}
#include "akregator_config_general.moc"
