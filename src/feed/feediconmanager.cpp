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

#include "akregator_debug.h"

#include <QUrl>

#include <QIcon>
#include <QMultiHash>
#include <QtDBus/QtDBus>

#include <cassert>
#include <QStandardPaths>

#define FAVICONINTERFACE "org.kde.FavIcon"

using namespace Akregator;

FaviconListener::~FaviconListener() {}

class Q_DECL_HIDDEN FeedIconManager::Private
{
    FeedIconManager *const q;
public:

    static FeedIconManager *m_instance;

    explicit Private(FeedIconManager *qq);
    ~Private();

    void loadIcon(const QString &url);
    QString iconLocation(const QUrl &) const;

    QHash<FaviconListener *, QString> m_listeners;
    QMultiHash<QString, FaviconListener *> urlDict;
    QDBusInterface *m_favIconsModule;
};

namespace
{

QString getIconUrl(const QUrl &url)
{
    return QLatin1String("http://") + url.host() + QLatin1Char('/');
}

}

FeedIconManager::Private::Private(FeedIconManager *qq) : q(qq)
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/FeedIconManager"), q, QDBusConnection::ExportScriptableSlots);
    m_favIconsModule = new QDBusInterface(QStringLiteral("org.kde.kded5"), QStringLiteral("/modules/favicons"), QStringLiteral(FAVICONINTERFACE));
    Q_ASSERT(m_favIconsModule);
    q->connect(m_favIconsModule, SIGNAL(iconChanged(bool,QString,QString)),
               q, SLOT(slotIconChanged(bool,QString,QString)));
}

FeedIconManager::Private::~Private()
{
    delete m_favIconsModule;
}

FeedIconManager *FeedIconManager::Private::m_instance = 0;

void FeedIconManager::Private::loadIcon(const QString &url_)
{
    const QUrl url(url_);

    QDBusPendingCall reply = m_favIconsModule->asyncCall(QStringLiteral("iconForUrl"), url.url());

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, q);

    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, q,
        [url, this](QDBusPendingCallWatcher *call){
            QDBusPendingReply<QString> reply = *call;
            if (reply.isError()) {
                m_favIconsModule->asyncCall(QStringLiteral("downloadHostIcon"), url.url());
                qCWarning(AKREGATOR_LOG) << "Couldn't reach favicon service. Request favicon for " << url << " failed:" << call->error().message();
            } else {
                q->slotIconChanged(false, url.host(), reply.argumentAt(0).toString());
            }
            call->deleteLater();
        }
    );
}

FeedIconManager *FeedIconManager::self()
{
    static FeedIconManager instance;
    if (!Private::m_instance) {
        Private::m_instance = &instance;
    }
    return Private::m_instance;
}

void FeedIconManager::addListener(const QUrl &url, FaviconListener *listener)
{
    Q_ASSERT(listener);
    removeListener(listener);
    const QString iconUrl = getIconUrl(url);
    d->m_listeners.insert(listener, iconUrl);
    d->urlDict.insert(iconUrl, listener);
    d->urlDict.insert(url.host(), listener);
    QMetaObject::invokeMethod(this, "loadIcon", Qt::QueuedConnection, Q_ARG(QString, iconUrl));
}

void FeedIconManager::removeListener(FaviconListener *listener)
{
    Q_ASSERT(listener);
    if (!d->m_listeners.contains(listener)) {
        return;
    }
    const QString url = d->m_listeners.value(listener);
    d->urlDict.remove(QUrl(url).host(), listener);
    d->urlDict.remove(url, listener);
    d->m_listeners.remove(listener);
}

FeedIconManager::FeedIconManager()
    : QObject()
    , d(new Private(this))
{
}

FeedIconManager::~FeedIconManager()
{
    delete d;
}

void FeedIconManager::slotIconChanged(bool isHost,
                                      const QString &hostOrUrl,
                                      const QString &iconName)
{
    Q_UNUSED(isHost);
    const QIcon icon(QStandardPaths::locate(QStandardPaths::CacheLocation, iconName + QLatin1String(".png")));
    Q_FOREACH (FaviconListener *const l, d->urlDict.values(hostOrUrl)) {
        l->setFavicon(icon);
    }
}

#include "moc_feediconmanager.cpp"
