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

#include <K4AboutData>
#include <KGenericFactory>
#include <KLocalizedString>
#include <kdemacros.h>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorAdvancedConfigFactory, registerPlugin<KCMAkregatorAdvancedConfig>();)
K_EXPORT_PLUGIN(KCMAkregatorAdvancedConfigFactory( "kcmakradvancedconfig" ))

KCMAkregatorAdvancedConfig::KCMAkregatorAdvancedConfig( QWidget* parent, const QVariantList& args )
    : KCModule( /*KCMAkregatorAdvancedConfigFactory::componentData(),*/ parent, args ), m_widget( new SettingsAdvanced )
{  
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_widget );
  #if 0 //QT5 
    K4AboutData *about = new K4AboutData( I18N_NOOP( "kcmakradvancedconfig" ), 0,
                                        ki18n( "Advanced Feed Reader Settings" ),
                                        0, KLocalizedString(), K4AboutData::License_GPL,
                                        ki18n( "(c), 2004 - 2008 Frank Osterfeld" ) );

    about->addAuthor( ki18n( "Frank Osterfeld" ), KLocalizedString(), "osterfeld@kde.org" );
    setAboutData( about );
#endif
    addConfig( Settings::self(), m_widget );
}


void KCMAkregatorAdvancedConfig::load()
{
    KCModule::load();
    m_widget->selectFactory(Settings::archiveBackend());
}

void KCMAkregatorAdvancedConfig::save()
{
    Settings::setArchiveBackend( m_widget->selectedFactory() );
    KCModule::save();
}

#include "akregator_config_advanced.moc"
