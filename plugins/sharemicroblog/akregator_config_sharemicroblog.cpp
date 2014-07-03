/*
    This file is part of Akregator.

    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>

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

#include "akregator_config_sharemicroblog.h"
#include "sharesettings.h"

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>
#include <kdemacros.h>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_FACTORY( KCMAkregatorShareConfigFactory, registerPlugin<KCMAkregatorShareConfig>(); )
K_EXPORT_PLUGIN( KCMAkregatorShareConfigFactory( "kcmakrshareconfig" ) )

KCMAkregatorShareConfig::KCMAkregatorShareConfig( QWidget* parent, const QVariantList& args )
: KCModule( /*KCMAkregatorShareConfigFactory::componentData(),*/ parent, args ),
    m_widget( new QWidget )
{
    ui.setupUi( m_widget );
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( m_widget );
    KAboutData *about = new KAboutData( QLatin1String( "kcmakrshareconfig" ),
                                        i18n( "Configure Share Services" ),
                                        QString(), QString(), KAboutLicense::GPL,
                                        i18n( "(c), 2004 - 2008 Frank Osterfeld" ) );

    about->addAuthor( i18n( "Frank Osterfeld" ), QString(), QStringLiteral("osterfeld@kde.org") );

    setAboutData( about );
    addConfig( ShareSettings::self(), m_widget );
}

KCMAkregatorShareConfig::~KCMAkregatorShareConfig()
{
}

void KCMAkregatorShareConfig::load()
{
    KCModule::load();

    KConfigGroup group( ShareSettings::self()->config(), "ShareService" );
    QStringList services;
    services << QLatin1String("https://identi.ca/api/");
    services << QLatin1String("https://twitter.com");
    ui.kcfg_ServiceUrl->clear();
    ui.kcfg_ServiceUrl->addItems(services);

    const QString serviceUrl = ShareSettings::serviceUrl();
    if ( !serviceUrl.isEmpty() )
        ui.kcfg_ServiceUrl->setEditText(serviceUrl);
}
#include "akregator_config_sharemicroblog.moc"

