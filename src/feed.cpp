/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "feed.h"
#include "feediconmanager.h"
#include "feedscollection.h"

#include <kurl.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <kapplication.h>

#include <qtimer.h>
#include <qlistview.h>
#include <qdom.h>

using namespace Akregator;
using namespace RSS;

Feed::Feed(QListViewItem *i, FeedsCollection *coll)
    : FeedGroup(i, coll)
    , updateTitle(false)
    , articles()
    , m_fetchError(false)
    , m_fetchTries(0)
    , m_merged(false)
    , m_unread(0)
{
}

Feed::~Feed()
{
}

bool Feed::isGroup()
{
    return false;
}

QDomElement Feed::toXml( QDomElement parent, QDomDocument document )
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    el.setAttribute( "title", title() );
    el.setAttribute( "xmlUrl", xmlUrl );
    el.setAttribute( "htmlUrl", htmlUrl );
    el.setAttribute( "description", description );
    el.setAttribute( "updateTitle", (updateTitle ? "true" : "false") );
    el.setAttribute( "autoFetch", (autoFetch() ? "true" : "false") );
    el.setAttribute( "fetchInterval", QString::number(fetchInterval()) );
    el.setAttribute( "type", "rss" ); // despite some additional fields, its still "rss" OPML
    el.setAttribute( "version", "RSS" );
    parent.appendChild( el );
    return el;
}

void Feed::dumpXmlData( QDomElement parent, QDomDocument doc )
{
    QDomElement channode = doc.createElement( "channel" );
    parent.appendChild(channode);
    QDomElement tnode = doc.createElement( "title" );
    QDomText t=doc.createTextNode( title() );
    tnode.appendChild(t);
    channode.appendChild(tnode);

    if (!htmlUrl.isEmpty())
    {
        QDomElement lnode = doc.createElement( "link" );
        QDomText ht=doc.createTextNode( htmlUrl );
        lnode.appendChild(ht);
        channode.appendChild(lnode);
    }

    // rss 2.0 requires channel description
    QDomElement dnode = doc.createElement( "description" );
    QDomText dt=doc.createTextNode( htmlUrl );
    dnode.appendChild(dt);
    channode.appendChild(dnode);

    ArticleSequence::ConstIterator it;
    ArticleSequence::ConstIterator en=articles.end();
    for (it = articles.begin(); it != en; ++it)
    {
        QDomElement enode = doc.createElement( "item" );
        (*it).dumpXmlData(enode, doc);
        channode.appendChild(enode);
    }

}

void Feed::markAllRead()
{
    ArticleSequence::Iterator it;
    ArticleSequence::Iterator en=articles.end();
    for (it = articles.begin(); it != en; ++it)
    {
        (*it).setStatus(MyArticle::Read);
    }
    m_unread=0;
}


void Feed::appendArticles(const Document &d, bool findDups)
{
    //kdDebug() << "appendArticles findDups=="<<findDups<< " isMerged=="<< m_merged<<endl;
    findDups=true;
    articles.enableSorting(false);
    Article::List::ConstIterator it;
    Article::List::ConstIterator en = d.articles().end();
    //kdDebug() << "m_unread before appending articles=="<<m_unread<<endl;
    for (it = d.articles().begin(); it != en; ++it)
    {
        MyArticle mya(*it);
        if (findDups)
        {
            ArticleSequence::ConstIterator oo=articles.find(mya);
            if (oo == articles.end() )
            {
                if (m_merged)
                    mya.setStatus(MyArticle::New);
                else
                {
                    if (mya.status()==MyArticle::New)
                        mya.setStatus(MyArticle::Unread);
                }
                appendArticle(mya);
            }
            //else{
            //kdDebug() << "got dup!!"<<mya.title()<<endl;
            //}
        }
        else
        {
            MyArticle mya(*it);
            if (!m_merged)
            {
                if (mya.status()==MyArticle::New)
                    mya.setStatus(MyArticle::Unread);
            }
            appendArticle(mya);
        }
    }
    articles.enableSorting(true);
    articles.sort();
    //kdDebug() << "m_unread after appending articles=="<<m_unread<<endl;
}

void Feed::appendArticle(const MyArticle &a)
{
    if (a.status()!=MyArticle::Read)
        m_unread++;
    articles.append(a);
}


void Feed::fetch(bool followDiscovery)
{
	m_followDiscovery=followDiscovery;
    m_fetchTries=0;

    // mark all new as unread
    ArticleSequence::Iterator it;
    ArticleSequence::Iterator en=articles.end();
    for (it = articles.begin(); it != en; ++it)
    {
        if ((*it).status()==MyArticle::New)
        {
            (*it).setStatus(MyArticle::Unread);
        }
    }

   // Disable icon to show it is fetching.
    if (!favicon.isNull())
    {
        KIconEffect iconEffect;
        QPixmap tempIcon = iconEffect.apply(favicon, KIcon::Small, KIcon::DisabledState);
        item()->setPixmap(0, tempIcon);
    }

    tryFetch();
}


void Feed::tryFetch()
{
    if (item() && m_fetchError)
        item()->setPixmap(0, KGlobal::iconLoader()->loadIcon("txt", KIcon::Small));

    m_fetchError=false;

    Loader *loader = Loader::create( this, SLOT(fetchCompleted(Loader *, Document, Status)) );
    loader->loadFrom( xmlUrl, new FileRetriever );
    // TODO: note that we probably don't want to load the favicon here enventually..
    QTimer::singleShot( 1000, this, SLOT(loadFavicon()) );
    //loadFavicon();
}

void Feed::fetchCompleted(Loader *l, Document doc, Status status)
{
    // Note that Loader::~Loader() is private, so you cannot delete Loader instances.
    // You don't need to do that anyway since Loader instances delete themselves.

    if (status!= Success)
    {
        if (m_followDiscovery && (status==ParseError) && (m_fetchTries < 3) && 			(l->discoveredFeedURL().isValid()))
        {
            m_fetchTries++;
            xmlUrl=l->discoveredFeedURL().url();
            emit fetchDiscovery(this);
            tryFetch();
            return;
        }
        else
        {
            faviconChanged(xmlUrl, KGlobal::iconLoader()->loadIcon("error", KIcon::Small));
            m_fetchError=true;
            emit fetchError(this);
            return;
        }
    }

    // Restore favicon.
    if (!favicon.isNull()) item()->setPixmap(0, favicon);

    m_fetchError=false;
    m_document=doc;
    //kdDebug() << "Feed fetched successfully [" << m_document.title() << "]" << endl;


    if (image.isNull())
    {
        QString u=xmlUrl;
        QString imageFileName=KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png";
        image=QPixmap(imageFileName, "PNG");

        if (image.isNull())
        {
            if (m_document.image()) // if we aint got teh image
                                    // and the feed provides one, get it....
            {
                connect (m_document.image(), SIGNAL(gotPixmap(const QPixmap &)),
                                   this, SLOT(imageChanged(const QPixmap &)));
                m_document.image()->getPixmap();
            }
        }
    }

    if (updateTitle || title().isEmpty()) setTitle( m_document.title() );

    description = m_document.description();
    htmlUrl = m_document.link().url();
    
    //kdDebug() << "ismerged reprots:::"<<isMerged()<<endl;

    bool findDups=isMerged();
    appendArticles(m_document, findDups);

    emit fetched(this);
}

void Feed::loadFavicon()
{
    FeedIconManager::self()->loadIcon(xmlUrl);
    connect (FeedIconManager::self(), SIGNAL(iconChanged(const QString &, const QPixmap &)),
                                this, SLOT(faviconChanged(const QString &, const QPixmap &)));
}

void Feed::faviconChanged(const QString &url, const QPixmap &p)
{
    if (xmlUrl==url && !m_fetchError)
    {
        if (item())
            item()->setPixmap(0, p);
        favicon=p;
    }
}

void Feed::imageChanged(const QPixmap &p)
{
    image=p;
    QString u=xmlUrl;
    image.save(KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png","PNG");
    emit(imageLoaded(this));
}

bool Feed::autoFetch() const
{
    return m_autoFetch;
}

int Feed::fetchInterval() const
{
    return m_fetchInterval;
}

void Feed::setAutoFetch(bool b)
{
    m_autoFetch=b;
}
void Feed::setFetchInterval(int i)
{
    m_fetchInterval=i;
}


// = ArticleSequence ===================================================== //

struct ArticleSequence::Private
{
   int dummy;
   bool doSort :1;
};

ArticleSequence::ArticleSequence()
   : MyArticle::List()
   , d(new Private)
{
}

ArticleSequence::ArticleSequence(const ArticleSequence &other)
   : MyArticle::List(other)
   , d(new Private)
{
}

ArticleSequence::~ArticleSequence()
{
   delete d;
}

/*
    The reason to include insert/append/prepend code here is to:
    a) check if there's another record with the exactly same pubDate() present,
    b) if so, adjust this inserted item's clock off by one second to keep sorting sane,
    c) re-sort added items (if enabled).
    d) use MyArticle::fetchDate for sorting! ( defined by MyArticle::operator <() )
 */
ArticleSequence::iterator ArticleSequence::insert( iterator it, const MyArticle &x )
{
    return MyArticle::List::insert( it, x );
}

void ArticleSequence::insert( iterator it, size_type n, const MyArticle &x )
{
    MyArticle::List::insert( it, n, x );
}

ArticleSequence::iterator ArticleSequence::append( const MyArticle &x )
{
    return MyArticle::List::append( x );
}

ArticleSequence::iterator ArticleSequence::prepend( const MyArticle &x )
{
    return MyArticle::List::prepend( x );
}


void ArticleSequence::enableSorting(bool b)
{
    d->doSort = b;
}

void ArticleSequence::sort()
{
    if (d->doSort)
    {
        qHeapSort( *this );
    }
}


#include "feed.moc"
