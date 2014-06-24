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
#include <K4AboutData>
#include <KGenericFactory>
#include <KLocalizedString>
#include <kdemacros.h>

#include <QButtonGroup>
#include <QVBoxLayout>

using namespace Akregator;

//QT5 K_PLUGIN_FACTORY(KCMAkregatorArchiveConfigFactory, registerPlugin<KCMAkregatorArchiveConfig>();)
K_EXPORT_PLUGIN(KCMAkregatorArchiveConfigFactory( "kcmakrarchiveconfig" ))

KCMAkregatorArchiveConfig::KCMAkregatorArchiveConfig( QWidget* parent, const QVariantList& args )
    : KCModule( /*KCMAkregatorArchiveConfigFactory::componentData(),*/ parent, args ), m_widget( new QWidget )
{
    Ui::SettingsArchive m_ui;
    m_ui.setupUi( m_widget );
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_widget );

    connect( m_ui.rb_LimitArticleNumber, SIGNAL(toggled(bool)),
             m_ui.kcfg_MaxArticleNumber, SLOT(setEnabled(bool)) );
    connect( m_ui.rb_LimitArticleAge, SIGNAL(toggled(bool)),
             m_ui.kcfg_MaxArticleAge, SLOT(setEnabled(bool)) );

    m_ui.kcfg_MaxArticleNumber->setSuffix(ki18ncp("Limit feed archive size to:", " article", " articles"));
    m_ui.kcfg_MaxArticleAge->setSuffix(ki18ncp("Delete articles older than:", " day", " days"));
    m_archiveModeGroup = new QButtonGroup( this );
    m_archiveModeGroup->addButton( m_ui.rb_KeepAllArticles, Settings::EnumArchiveMode::keepAllArticles );
    m_archiveModeGroup->addButton( m_ui.rb_LimitArticleNumber, Settings::EnumArchiveMode::limitArticleNumber );
    m_archiveModeGroup->addButton( m_ui.rb_LimitArticleAge, Settings::EnumArchiveMode::limitArticleAge );
    m_archiveModeGroup->addButton( m_ui.rb_DisableArchiving, Settings::EnumArchiveMode::disableArchiving );
    connect( m_archiveModeGroup, SIGNAL(buttonClicked(int)), this, SLOT(changed()) );
#if 0 //QT5
    K4AboutData *about = new K4AboutData( I18N_NOOP( "kcmakrarchiveconfig" ), 0,
                                        ki18n( "Configure Feed Reader Archive" ),
                                        0, KLocalizedString(), K4AboutData::License_GPL,
                                        ki18n( "(c), 2004 - 2008 Frank Osterfeld" ) );

    about->addAuthor( ki18n( "Frank Osterfeld" ), KLocalizedString(), "osterfeld@kde.org" );
    setAboutData( about );
#endif
    addConfig( Settings::self(), m_widget );
}

void KCMAkregatorArchiveConfig::load()
{
    setArchiveMode( Settings::archiveMode() );
    KCModule::load();
}

void KCMAkregatorArchiveConfig::save()
{
    Settings::setArchiveMode( archiveMode() );
    KCModule::save();
}


void KCMAkregatorArchiveConfig::setArchiveMode( int mode )
{
    QAbstractButton* const b = m_archiveModeGroup->button( mode );
    if ( b )
        b->setChecked( true );
    else
        kWarning( "No button for %d registered, ignoring call", mode );
}

int KCMAkregatorArchiveConfig::archiveMode() const {
    const int id = m_archiveModeGroup->checkedId();
    if ( id < 0 || id >= Settings::EnumArchiveMode::COUNT )
        return Settings::EnumArchiveMode::keepAllArticles;
    return id;
}

