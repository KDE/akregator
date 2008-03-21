/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "feediconmanager.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <k3staticdeleter.h>
#include <kurl.h>

#include <QIcon>
#include <QList>
#include <QMultiHash>
#include <QPixmap>
#include <QtDBus/QtDBus>

#include <cassert>

#define FAVICONINTERFACE "org.kde.FavIcon"


using namespace Akregator;

FaviconListener::~FaviconListener() {}

class FeedIconManager::Private
{
    FeedIconManager* const q;
public:
    
    static FeedIconManager* m_instance;

    explicit Private( FeedIconManager* qq );
    ~Private();
    

    void loadIcon( const QString& url );
    QString iconLocation( const KUrl& ) const;
    
    QHash<FaviconListener*,QString> m_listeners;
    QMultiHash<QString, FaviconListener*> urlDict;
    QDBusInterface *m_favIconsModule;
};

namespace {

QString getIconUrl( const KUrl& url )
{
    return "http://" + url.host() + '/';
}

}

FeedIconManager::Private::Private( FeedIconManager* qq ) : q( qq )
{
    QDBusConnection::sessionBus().registerObject("/FeedIconManager", q, QDBusConnection::ExportScriptableSlots);
    m_favIconsModule = new QDBusInterface("org.kde.kded", "/modules/favicons", FAVICONINTERFACE);
    Q_ASSERT( m_favIconsModule );
    q->connect( m_favIconsModule, SIGNAL( iconChanged( bool, QString, QString ) ),
                q, SLOT( slotIconChanged( bool, QString, QString ) ) );
}

FeedIconManager::Private::~Private()
{
    delete m_favIconsModule;
}

FeedIconManager *FeedIconManager::Private::m_instance = 0;


QString FeedIconManager::Private::iconLocation(const KUrl & url) const
{
    QDBusReply<QString> reply = m_favIconsModule->call( "iconForUrl", url.url() );
    return reply.isValid() ? reply.value() : QString();
}


void FeedIconManager::Private::loadIcon( const QString & url_ )
{
    const KUrl url(url_);

    QString iconFile = iconLocation( url );

    if (iconFile.isNull())
        m_favIconsModule->call( "downloadHostIcon", url.url() );
    else
        q->slotIconChanged( false, url.url(), iconFile );
}

static K3StaticDeleter<FeedIconManager> feediconmanagersd;

FeedIconManager* FeedIconManager::self()
{
    if (!Private::m_instance)
        Private::m_instance = feediconmanagersd.setObject(Private::m_instance, new FeedIconManager);
    return Private::m_instance;
}

void FeedIconManager::addListener( const KUrl& url, FaviconListener* listener )
{
    assert( listener );
    removeListener( listener );
    const QString iconUrl = getIconUrl( url );
    d->m_listeners.insert( listener, iconUrl );
    d->urlDict.insert( iconUrl, listener );
    QMetaObject::invokeMethod( this, SLOT( loadIcon( QString ) ), Q_ARG( QString, iconUrl ) );
}

void FeedIconManager::removeListener( FaviconListener* listener )
{
    assert( listener );
    if ( !d->m_listeners.contains( listener ) )
        return;
    const QString url = d->m_listeners.value( listener );
    d->urlDict.remove( url, listener );
    d->m_listeners.remove( listener );
}

FeedIconManager::FeedIconManager()
:  QObject(), d( new Private( this ) )
{
}

FeedIconManager::~FeedIconManager()
{
    delete d;
}

void FeedIconManager::slotIconChanged( bool isHost,
                                       const QString& hostOrUrl,
                                       const QString& iconName )
{
    Q_UNUSED( isHost );
    const QIcon icon( KGlobal::dirs()->findResource( "cache", iconName+".png" ) );
    Q_FOREACH( FaviconListener* l, d->urlDict.values( hostOrUrl ) )
        l->setFavicon( icon );
}

#include "feediconmanager.moc"
