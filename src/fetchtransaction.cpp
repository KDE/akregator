/***************************************************************************
 *  Copyright (C) 2004 by Sashmit Bhaduri (smt@vfemail.net)                *
 *                                                                         *
 *  Licensed under GPL.                                                    *
 ***************************************************************************/

#include <kdebug.h>

#include "fetchtransaction.h"
#include "feediconmanager.h"
#include "akregatorconfig.h"
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
    if (m_fetchList.count() == 0)
    {
        m_running = false;
        emit completed();
    }
    
    int i = 0;
    m_running = true;
    m_totalFetches=m_fetchList.count();
    m_fetchesDone=0;
    
    while (i < m_concurrentFetches)
    {
        doFetch(0);
        i++;
    }
}

void FetchTransaction::stop()
{
    if (!m_running)
        return;
    
    m_running=false;
    Feed *f;
    for (f=m_currentFetches.first(); f; f=m_currentFetches.next())
        f->abortFetch();

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
    
    connect (f, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetched(Feed*)));
    connect (f, SIGNAL(fetchError(Feed*)), this, SLOT(slotFeedError(Feed*)));

    m_fetchList.append(f);
}

void FetchTransaction::doFetch(int c)
{
    Feed *f=m_fetchList.at(c);
    if (!f) return;
    //kdDebug() << "starting fetch: "<<f->title()<<endl;
    f->fetch(false, this);
    m_currentFetches.append(f);
    m_fetchList.remove(c);
}

void FetchTransaction::slotFeedFetched(Feed *f)
{
    if (!m_running)
        return;

    m_fetchesDone++;
    emit fetched(f);
    feedDone(f);
}

void FetchTransaction::slotFeedError(Feed *f)
{
    if (!m_running)
        return;

    m_fetchesDone++;
    emit fetchError(f);
    feedDone(f);
}

void FetchTransaction::feedDone(Feed *f)
{
    //kdDebug() << "feed done: "<<f->title()<<endl;
    disconnect (f, SIGNAL(fetched(Feed*)), this, SLOT(slotFeedFetched(Feed*)));
    disconnect (f, SIGNAL(fetchError(Feed*)), this, SLOT(slotFeedError(Feed*)));

    m_currentFetches.remove(f);

    if (m_fetchList.isEmpty() && m_currentFetches.isEmpty())
    {
        startFetchImages();
        startFetchIcons();
        m_running = false;
        emit completed();
        return;
    }

    doFetch(0);
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

void FetchTransaction::doFetchIcon(int c)
{
    Feed *f=m_iconFetchList.at(c);
    if (!f) return;
    KURL u(f->xmlUrl());
    QString h=u.host();
    m_iconFetchList.remove(c);
    FeedIconManager::self()->loadIcon("http://"+h+"/");
}

void FetchTransaction::startFetchIcons()
{
    int i=0;
    m_running=true;

    while (i < m_concurrentFetches)
    {
        doFetchIcon(0);
        i++;
    }
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

    doFetchIcon(0);
}

void FetchTransaction::loadImage(Feed *f, Image *i)
{
    if (!m_imageFetchDict.find(i))
        m_imageFetchList.append(i);

    m_imageFetchDict.insert(i, f);

    connect (i, SIGNAL(gotPixmap(const QPixmap &)),
             this, SLOT(slotImageFetched(const QPixmap &)));
}

void FetchTransaction::doFetchImage(int c)
{
    Image *i=m_imageFetchList.at(c);
    if (!i) return;

    m_currentImageFetches.append(i);
    i->getPixmap();
    m_imageFetchList.remove(c);
}

void FetchTransaction::startFetchImages()
{
    int i=0;
    m_running=true;

    while (i < m_concurrentFetches)
    {
        doFetchImage(0);
        i++;
    }
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
    doFetchImage(0);
}

#include "fetchtransaction.moc"

// vim: set et ts=4 sts=4 sw=4:

