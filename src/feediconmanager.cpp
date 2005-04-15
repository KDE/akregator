/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "feed.h"
#include "feediconmanager.h"

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <kurl.h>

#include <qpixmap.h>

using namespace Akregator;

FeedIconManager *FeedIconManager::m_instance = 0;

static KStaticDeleter<FeedIconManager> feediconmanagersd;

FeedIconManager* FeedIconManager::self()
{
    if (!m_instance)
        m_instance = feediconmanagersd.setObject(m_instance, new FeedIconManager);
    return m_instance;
}

void FeedIconManager::fetchIcon(Feed* feed)
{
    if (!m_registeredFeeds.contains(feed))
    {
        m_registeredFeeds.append(feed);
        connect(feed, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotFeedDestroyed(TreeNode*)));
    }
    QString iconURL = getIconURL(KURL(feed->xmlUrl()));
    m_urlDict.insert(iconURL, feed);
    loadIcon(iconURL);
}

FeedIconManager::FeedIconManager(QObject * parent, const char *name)
:  QObject(parent, name), DCOPObject("FeedIconManager")
{
    connectDCOPSignal("kded",
                      "favicons", "iconChanged(bool, QString, QString)",
                      "slotIconChanged(bool, QString, QString)", false);
}


FeedIconManager::~FeedIconManager()
{}

void FeedIconManager::loadIcon(const QString & url)
{
    KURL u(url);

    QString iconFile = iconLocation(u);
    
    if (iconFile.isNull())
    {
        QByteArray data;
        QDataStream ds(data, IO_WriteOnly);
        ds << u;
        kapp->dcopClient()->send("kded", "favicons", "downloadHostIcon(KURL)",
                                 data);
    }
    else
        slotIconChanged(false, url, iconFile);

}

QString FeedIconManager::getIconURL(const KURL& url)
{
    return "http://" +url.host() + "/";
}

QString FeedIconManager::iconLocation(const KURL & url) const
{
    QByteArray data, reply;
    QCString replyType;
    QDataStream ds(data, IO_WriteOnly);

    ds << url;

    kapp->dcopClient()->call("kded", "favicons", "iconForURL(KURL)", data,
                             replyType, reply);

    if (replyType == "QString") {
        QDataStream replyStream(reply, IO_ReadOnly);
        QString result;
        replyStream >> result;
        return result;
    }

    return QString::null;
}

void FeedIconManager::slotFeedDestroyed(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);
    if (feed)
        while (m_registeredFeeds.contains(feed))
            m_registeredFeeds.remove(m_registeredFeeds.find(feed));
}

void FeedIconManager::slotIconChanged(bool /*isHost*/, const QString& hostOrURL,
                                  const QString& iconName)
{
    QString iconFile = KGlobal::dirs()->findResource("cache",
                                 iconName+".png");
    Feed* f;
    while (f = m_urlDict.take(hostOrURL))
        if (m_registeredFeeds.contains(f))
            f->setFavicon(QPixmap(iconFile));
    emit signalIconChanged(hostOrURL, iconFile);
}

#include "feediconmanager.moc"
