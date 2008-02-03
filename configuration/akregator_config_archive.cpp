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

#include "akregator_config_archive.h"
#include "akregatorconfig.h"

#include "ui_settings_archive.h"

#include <KAboutData>
#include <KConfigDialogManager>
#include <KGenericFactory>
#include <KLocalizedString>
#include <kdemacros.h>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorArchiveConfigFactory, registerPlugin<KCMAkregatorArchiveConfig>();)
K_EXPORT_PLUGIN(KCMAkregatorArchiveConfigFactory( "kcmakrarchiveconfig" ))

KCMAkregatorArchiveConfig::KCMAkregatorArchiveConfig( QWidget* parent, const QVariantList& args )
    : KCModule( KCMAkregatorArchiveConfigFactory::componentData(), parent, args ), m_widget( new QWidget )
{  
    Ui::SettingsArchive ui;
    ui.setupUi( m_widget );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_widget );
   
    connect( ui.rb_LimitArticleNumber, SIGNAL( toggled( bool ) ),
             ui.kcfg_MaxArticleNumber, SLOT( setEnabled( bool ) ) );
    connect( ui.rb_LimitArticleAge, SIGNAL( toggled( bool ) ),
             ui.kcfg_MaxArticleAge, SLOT( setEnabled( bool ) ) );

    KAboutData *about = new KAboutData( I18N_NOOP( "kcmakrarchiveconfig" ), 0,
                                        ki18n( "Configure Feed Reader Archive" ),
                                        0, KLocalizedString(), KAboutData::License_GPL,
                                        ki18n( "(c), 2004 - 2008 Frank Osterfeld" ) );

    about->addAuthor( ki18n( "Frank Osterfeld" ), KLocalizedString(), "osterfeld@kde.org" );
    setAboutData( about );

    addConfig( Settings::self(), m_widget );
}

#include "akregator_config_archive.moc"
