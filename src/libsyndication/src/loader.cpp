/*
 * loader.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#include "dataretriever.h"
#include "documentsource.h"
#include "feed.h"
#include "loader.h"
#include "parsercollection.h"

#include <ksharedptr.h>
#include <kurl.h>

#include <QBuffer>
#include <QRegExp>
#include <QStringList>

namespace LibSyndication {

struct Loader::LoaderPrivate
{
    LoaderPrivate() : retriever(0), lastError(Success)
    {
    }
    
    ~LoaderPrivate()
    {
        delete retriever;
    }
    
    DataRetriever* retriever;
    LibSyndication::ErrorCode lastError;
    KUrl discoveredFeedURL;
    KUrl url;
};

Loader* Loader::create()
{
    return new Loader;
}

Loader *Loader::create(QObject* object, const char* slot)
{
    Loader *loader = create();
    connect(loader, SIGNAL(loadingComplete(LibSyndication::Loader*,
            LibSyndication::FeedPtr, LibSyndication::ErrorCode)),
            object, slot);
    return loader;
}

Loader::Loader() : d(new LoaderPrivate)
{
}

Loader::~Loader()
{
    delete d;
}

void Loader::loadFrom(const KUrl& url)
{
    loadFrom(url, new FileRetriever);
}

void Loader::loadFrom(const KUrl &url, DataRetriever *retriever)
{
    if (d->retriever != 0L)
        return;

    d->url = url;
    d->retriever = retriever;

    connect(d->retriever, SIGNAL(dataRetrieved(const QByteArray&, bool)),
            this, SLOT(slotRetrieverDone(const QByteArray&, bool)));

    d->retriever->retrieveData(url);
}

LibSyndication::ErrorCode Loader::errorCode() const
{
    return d->lastError;
}

void Loader::abort()
{
    if (d && d->retriever)
    {
        d->retriever->abort();
        delete d->retriever;
        d->retriever = 0L;
    }
    
    emit loadingComplete(this, 0L, Aborted);
    delete this;
}

const KUrl &Loader::discoveredFeedURL() const
{
    return d->discoveredFeedURL;
}

void Loader::slotRetrieverDone(const QByteArray& data, bool success)
{
    //d->lastError = d->retriever->errorCode(); // TODO
    
    delete d->retriever;
    d->retriever = 0;
    
    FeedPtr feed;
    ErrorCode status = Success;
    
    if (success)
    {
        DocumentSource src(data, d->url.url());
        feed = ParserCollection::self()->parse(src);
        
        if (ParserCollection::self()->lastError() != LibSyndication::Success)
        {
            status = ParserCollection::self()->lastError();
            discoverFeeds(data);
        }
    }
    else
    {
        status = FileNotFound; // TODO: correctly set error code
    }

   emit loadingComplete(this, feed, status);

   delete this;
}

void Loader::discoverFeeds(const QByteArray &data)
{
    QString str = QString(data).simplified();
    QString s2;
    //QTextStream ts( &str, QIODevice::WriteOnly );
    //ts << data.data();

    // "<[\\s]link[^>]*rel[\\s]=[\\s]\\\"[\\s]alternate[\\s]\\\"[^>]*>"
    // "type[\\s]=[\\s]\\\"application/rss+xml\\\""
    // "href[\\s]=[\\s]\\\"application/rss+xml\\\""
    QRegExp rx( "(?:REL)[^=]*=[^sAa]*(?:service.feed|ALTERNATE)[\\s]*[^s][^s](?:[^>]*)(?:HREF)[^=]*=[^A-Z0-9-_~,./$]*([^'\">\\s]*)", false);
    if (rx.search(str)!=-1)
        s2=rx.cap(1);
    else{
    // does not support Atom/RSS autodiscovery.. try finding feeds by brute force....
        int pos=0;
        QStringList feeds;
        QString host=d->url.host();
        rx.setPattern("(?:<A )[^H]*(?:HREF)[^=]*=[^A-Z0-9-_~,./]*([^'\">\\s]*)");
        while ( pos >= 0 ) {
            pos = rx.search( str, pos );
            s2=rx.cap(1);
            if (s2.endsWith(".rdf")|s2.endsWith(".rss")|s2.endsWith(".xml"))
                    feeds.append(s2);
            if ( pos >= 0 ) {
                pos += rx.matchedLength();
            }
        }

        s2=feeds.first();
        KUrl testURL;
        // loop through, prefer feeds on same host
        QStringList::Iterator end( feeds.end() );
        for ( QStringList::Iterator it = feeds.begin(); it != end; ++it ) {
            testURL=*it;
            if (testURL.host()==host)
            {
                s2=*it;
                break;
            }
        }
    }

    if (s2.isNull()) 
    {
        return;
    }

    if (KUrl::isRelativeURL(s2))
    {
        if (s2.startsWith("//"))
        {
            s2=s2.prepend(d->url.protocol()+":");
            d->discoveredFeedURL=s2;
        }
        else if (s2.startsWith("/"))
        {
            d->discoveredFeedURL=d->url;
            d->discoveredFeedURL.setPath(s2);
        }
        else
        {
            d->discoveredFeedURL=d->url;
            d->discoveredFeedURL.addPath(s2);
        }
        d->discoveredFeedURL.cleanPath();
    }
    else
        d->discoveredFeedURL=s2;

    d->discoveredFeedURL.cleanPath();
}

} // namespace LibSyndication

#include "loader.moc"
