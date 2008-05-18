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

#include "akregator_config_onlinesync.h"
#include "onlinesyncsettings.h"

#include "ui/configurationwidget.h"

#include <KAboutData>
#include <KConfigDialogManager>
#include <KGenericFactory>
#include <KLocalizedString>
#include <kdemacros.h>

#include <QVBoxLayout>

using namespace Akregator;
using namespace feedsync;

K_PLUGIN_FACTORY( KCMAkregatorOnlineSyncConfigFactory, registerPlugin<KCMAkregatorOnlineSyncConfig>(); )
K_EXPORT_PLUGIN( KCMAkregatorOnlineSyncConfigFactory( "kcmakronlinesyncconfig" ) )

KCMAkregatorOnlineSyncConfig::KCMAkregatorOnlineSyncConfig( QWidget* parent, const QVariantList& args )
    : KCModule( KCMAkregatorOnlineSyncConfigFactory::componentData(), parent, args ), m_widget( new ConfigurationWidget )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_widget );

    KAboutData* about = new KAboutData( I18N_NOOP( "kcmakronlinesyncconfig" ), 0,
                                        ki18n( "Configure Online Readers" ),
                                        0, KLocalizedString(), KAboutData::License_GPL,
                                        ki18n( "(c), 2008 Didier Hoarau" ) );

    about->addAuthor( ki18n( "Didier Hoarau" ), KLocalizedString(), "did.hoarau@gmail.com" );
    setAboutData( about );

    addConfig( OnlineSyncSettings::self(), m_widget );
}

KCMAkregatorOnlineSyncConfig::~KCMAkregatorOnlineSyncConfig()
{
}


#include "akregator_config_onlinesync.moc"
