/***************************************************************************
 *  Copyright (C) 2004 by Sashmit Bhaduri (smt@vfemail.net)                *
 *                                                                         *
 *  Licensed under GPL.                                                    *
 ***************************************************************************/

#include <kdebug.h>

#include <qtimer.h>

#include "fetchtransaction.h"
#include "feediconmanager.h"
#include "akregatorconfig.h"
#include "treenode.h"
#include "feed.h"

using namespace Akregator;

FetchTransaction::FetchTransaction(QObject *parent): QObject(parent, "transaction"),
    m_fetchList(), m_currentFetches(), m_iconFetchList(), m_iconFetchDict(),
    m_imageFetchList(), m_currentImageFetches(), m_imageFetchDict(), m_totalFetches(0)
{
    m_concurrentFetches=Settings::concurrentFetches();
    m_running=false;

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
        QTimer::singleShot(0, this, SLOT(slotFetchNextFeed()));
}

void FetchTransaction::stop()
{
    if (!m_running)
        return;
    
    m_running=false;
    Feed *f;
    for (f=m_currentFetches.first(); f; f=m_currentFetches.next())
        f->slotAbortFetch();

    Image *i;
    for (i=m_currentImageFetches.first(); i; i=m_currentImageFetches.next())
        i->abort();
    
    m_currentFetches.clear();
    m_currentImageFetches.clear();
    m_fetchList.clear();
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
    //kdDebug() << "feed done: "<<f->title()<<endl;
    disconnectFromFeed(f);    
    m_currentFetches.remove(f);
    m_fetchList.remove(f);
    
    if (m_fetchList.isEmpty() && m_currentFetches.isEmpty())
    {
        startFetchImages();
        startFetchIcons();
        m_running = false;
        emit completed();
        return;
    }

    QTimer::singleShot(0, this, SLOT(slotFetchNextFeed()));
}

void FetchTransaction::loadIcon(Feed *f)
{
    KURL u(f->xmlUrl());
    if (u.protocol()!= "http")
        return;
    QString h="http://"+u.host();
    if (!m_iconFetchDict.find(h))
        m_iconFetchList.append(f);

    m_iconFetchDict.insert(h, f);
}

void FetchTransaction::slotFetchNextIcon()
{
    Feed *f = m_iconFetchList.at(0);
    if (!f)
        return;
    
    KURL u(f->xmlUrl());
    QString h = u.host();
    m_iconFetchList.remove((uint)0);
    FeedIconManager::self()->loadIcon("http://"+h+"/");
}

void FetchTransaction::startFetchIcons()
{
    m_running = true;
    
    for (int i = 0; i < m_concurrentFetches; ++i)
        QTimer::singleShot(0, this, SLOT(slotFetchNextIcon()));
}

void FetchTransaction::slotFaviconFetched(const QString &host, const QPixmap &p)
{
    QString h=host;
    if (h.left(6) != "http://")
        h="http://"+h;
    Feed *f=m_iconFetchDict[h];
    while (f)
    {
        f->setFavicon(p);
        m_iconFetchDict.remove(h);
        f=m_iconFetchDict[h];
    }

    QTimer::singleShot(0, this, SLOT(slotFetchNextIcon()));
}

void FetchTransaction::loadImage(Feed *f, Image *i)
{
    if (!m_imageFetchDict.find(i))
        m_imageFetchList.append(i);

    m_imageFetchDict.insert(i, f);

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
    m_running = true;

    for (int i = 0; i < m_concurrentFetches; ++i)
        QTimer::singleShot(0, this, SLOT(slotFetchNextImage()));
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
    
    QTimer::singleShot(0, this, SLOT(slotFetchNextImage()));
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
    disconnect (feed, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotNodeDestroyed(TreeNode*)));
}

void FetchTransaction::slotNodeDestroyed(TreeNode* node)
{
    Feed* feed = dynamic_cast<Feed*>(node);

    if (!feed)
        return;

    m_fetchList.remove(feed);
    m_iconFetchList.remove(feed);
    m_imageFetchDict.remove(feed);
    
    disconnectFromFeed(feed);
}

#include "fetchtransaction.moc"

// vim: set et ts=4 sts=4 sw=4:

