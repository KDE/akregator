/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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

#include <kdebug.h>

#include "akregatorconfig.h"
#include "feediconmanager.h"
#include "fetchtransaction.h"
#include "feed.h"
#include "treenode.h"

using namespace Akregator;

FetchTransaction::FetchTransaction(QObject *parent): QObject(parent, "transaction"),
    m_fetchList(), m_currentFetches(), m_iconFetchList(), m_iconFetchDict(),
    m_imageFetchList(), m_currentImageFetches(), m_imageFetchDict(), m_totalFetches(0), m_running(false)
{
    m_concurrentFetches=Settings::concurrentFetches();

    connect (FeedIconManager::self(), SIGNAL(iconChanged(const QString &, const QPixmap &)), this, SLOT(slotFaviconFetched(const QString &, const QPixmap &)));
}

FetchTransaction::~FetchTransaction()
{
    stop();
}

void FetchTransaction::start()
{
    if (m_running)
        return;
    
    if (m_fetchList.count() == 0)
    {
        m_running = false;
        emit completed();
    }
    
    m_running = true;
    m_totalFetches=m_fetchList.count();
    m_fetchesDone=0;
    
    for (int i = 0; i < m_concurrentFetches; ++i)
        slotFetchNextFeed();
}

void FetchTransaction::stop()
{
    if (!m_running)
        return;
    
    Feed *f;
    for (f=m_currentFetches.first(); f; f=m_currentFetches.next())
        f->slotAbortFetch();

    for (Image* i=m_currentImageFetches.first(); i; i=m_currentImageFetches.next())
        i->abort();

    m_running = false;   
    clear();
}

void FetchTransaction::clear()
{
    if (m_running)
        return;

    m_fetchList.clear();
    m_currentFetches.clear();
    

    m_currentImageFetches.clear();
    m_imageFetchList.clear();
    
    m_currentIconFetches.clear();
    m_iconFetchList.clear();
    m_iconFetchDict.clear();
        
    m_fetchesDone = 0;
    m_totalFetches = 0;    
}

void FetchTransaction::addFeed(Feed *f)
{
    connectToFeed(f);
    m_fetchList.append(f);
}

void FetchTransaction::slotFetchNextFeed()
{
    Feed *f = m_fetchList.at(0);
    if (!f)
        return;
    //kdDebug() << "starting fetch: " << f << endl;
    f->fetch(false, this);
    m_currentFetches.append(f);
    m_fetchList.remove((uint)0);
}

void FetchTransaction::slotFeedFetched(Feed *f)
{
    if (!m_running)
        return;

    m_fetchesDone++;
    emit fetched(f);
    feedDone(f);
}

void FetchTransaction::slotFetchError(Feed *f)
{
    if (!m_running)
        return;

    m_fetchesDone++;
    emit fetchError(f);
    feedDone(f);
}

void FetchTransaction::slotFetchAborted(Feed *f)
{
    if (!m_running)
        return;

    m_fetchesDone++;
    emit fetched(f); // FIXME: better use a signal like signalAborted(Feed*)
    feedDone(f);
}


void FetchTransaction::feedDone(Feed *f)
{
    if (f)
    {
        disconnectFromFeed(f);    
        m_currentFetches.remove(f);
        m_fetchList.remove(f);
        slotFetchNextFeed();
    }
    
    if (m_fetchList.isEmpty() && m_currentFetches.isEmpty())
    {
        startFetchImages();
        startFetchIcons();
        m_running = false;
        emit completed();
    }
}

void FetchTransaction::addIcon(Feed *f)
{
    KURL u(f->xmlUrl());
    if (u.protocol()!= "http")
        return;
    QString h="http://"+u.host();
    if (!m_iconFetchDict.find(h))
        m_iconFetchList.append(f);
    
    m_iconFetchDict.insert(h, f);
    connectToFeed(f);
}

void FetchTransaction::slotFetchNextIcon()
{
    Feed *f = m_iconFetchList.at(0);
    if (!f)
        return;
    kdDebug() << "FT::sFNextIcon(): accessing " << f << endl;
    KURL u(f->xmlUrl());
    QString h = u.host();
    m_iconFetchList.remove((uint)0);
    m_currentIconFetches.append(f);
    FeedIconManager::self()->loadIcon("http://"+h+"/");
}

void FetchTransaction::startFetchIcons()
{
    for (int i = 0; i < m_concurrentFetches; ++i)
        slotFetchNextIcon();
}

void FetchTransaction::slotFaviconFetched(const QString &host, const QPixmap &p)
{
    QString h=host;
    if (h.left(6) != "http://")
        h="http://"+h;
    
    Feed *f = m_iconFetchDict[h];
        
    while (f)
    {
        m_iconFetchDict.remove(h);
        if (m_currentIconFetches.contains(f)) // if false, the feed was deleted
        {
            m_currentIconFetches.remove(f);
            f->setFavicon(p);
        }
        
        f = m_iconFetchDict[h];
    }

    slotFetchNextIcon();
}

void FetchTransaction::addImage(Feed *f, Image *i)
{
    if (!m_imageFetchDict.find(i))
        m_imageFetchList.append(i);

    m_imageFetchDict.insert(i, f);
    connectToFeed(f);
    
    connect (i, SIGNAL(gotPixmap(const QPixmap &)),
             this, SLOT(slotImageFetched(const QPixmap &)));
}

void FetchTransaction::slotFetchNextImage()
{
    Image *i=m_imageFetchList.at(0);
    if (!i) return;

    m_currentImageFetches.append(i);
    i->getPixmap();
    m_imageFetchList.remove((uint)0);
}

void FetchTransaction::startFetchImages()
{
    for (int i = 0; i < m_concurrentFetches; ++i)
        slotFetchNextImage();
}

void FetchTransaction::slotImageFetched(const QPixmap &p)
{
    const Image *i=static_cast<const Image*>(sender());
    if (!i) return;

    Feed *f=m_imageFetchDict[(Image*)i];
    while (f)
    {
        f->setImage(p);
        m_imageFetchDict.remove((Image*)i);
        f=m_imageFetchDict[(Image*)i];
    }
    m_currentImageFetches.remove((Image*)i);
    
    slotFetchNextImage();
}

void FetchTransaction::connectToFeed(Feed* feed)
{
    connect (feed, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetched(Feed*)));
    connect (feed, SIGNAL(fetchError(Feed*)), this, SLOT(slotFetchError(Feed*)));
    connect (feed, SIGNAL(fetchAborted(Feed*)), this, SLOT(slotFetchAborted(Feed*)));
    connect (feed, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*)));
}

void FetchTransaction::disconnectFromFeed(Feed* feed)
{
    disconnect (feed, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetched(Feed*)));
    disconnect (feed, SIGNAL(fetchError(Feed*)), this, SLOT(slotFetchError(Feed*)));
    disconnect (feed, SIGNAL(fetchAborted(Feed*)), this, SLOT(slotFetchAborted(Feed*)));

    // NOTE: Don't disconnect from signalDestroyed(), since that leads
    // to problems when the feed is deleted after RSS fetching
    // (and would be disconnected) but before Icon/Image fetching. This will
    // lead to some unnecessary slotNodeDestroyed() calls, but since feeds
    // are deleted only rarely, we can live with that.
    
//    disconnect (feed, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*)));
}

void FetchTransaction::slotNodeDestroyed(TreeNode* node)
{
    Feed* feed = static_cast<Feed*> (node);

    if (!feed)
        return;

     // remove all occurrences of this feed
    while (m_fetchList.remove(feed)) /** do nothing */;
    while (m_iconFetchList.remove(feed)) /** do nothing */;
    while (m_currentIconFetches.remove(feed)) /** do nothing */;
    while (m_imageFetchDict.remove(feed)) /** do nothing */;
}

#include "fetchtransaction.moc"

// vim: set et ts=4 sts=4 sw=4:

