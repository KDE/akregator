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

#include <qtimer.h>
#include <qlistview.h>
#include <qdom.h>

using namespace Akregator;
using namespace RSS;

Feed::Feed(QListViewItem *i, FeedsCollection *coll)
    : FeedGroup(i, coll)
    , isLiveJournal(false)
    , ljAuthMode(AuthNone)
    , updateTitle(false)
    , articles()
    , m_fetchError(false)
{
}

Feed::~Feed()
{
}

bool Feed::isGroup()
{
    return false;
}

QString Feed::ljAuthModeStr()
{
    if (ljAuthMode == AuthLocal)
        return "local";
    if (ljAuthMode == AuthGlobal)
        return "global";
    return "none";
}

/*static*/ Feed::LJAuthMode Feed::authModeFromString(const QString &mode)
{
    QString m = mode.lower();
    if (m == "local")
        return AuthLocal;
    if (m == "global")
        return AuthGlobal;
    return AuthNone;
}


QDomElement Feed::toXml( QDomElement parent, QDomDocument document )
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    el.setAttribute( "title", title() );
    el.setAttribute( "xmlUrl", xmlUrl );
    el.setAttribute( "htmlUrl", htmlUrl );
    el.setAttribute( "description", description );
    el.setAttribute( "isLiveJournal", (isLiveJournal ? "true" : "false") );
    el.setAttribute( "ljUserName", ljUserName );
    el.setAttribute( "ljAuthMode", ljAuthModeStr() );
    el.setAttribute( "ljLogin", ljLogin );
    el.setAttribute( "ljPassword", ljPassword );
    el.setAttribute( "updateTitle", (updateTitle ? "true" : "false") );
    el.setAttribute( "type", "rss" ); // despite some additional fields, its still "rss" OPML
    el.setAttribute( "version", "RSS" );
    parent.appendChild( el );
    return el;
}

void Feed::fetch()
{
    m_fetchError=false;
    Loader *loader = Loader::create( this, SLOT(fetchCompleted(Loader *, Document, Status)) );
    loader->loadFrom( xmlUrl, new FileRetriever );

    // TODO: note that we probably don't want to load the favicon here enventually..
    QTimer::singleShot( 2000, this, SLOT(loadFavicon()) );
    loadFavicon();
}

void Feed::fetchCompleted(Loader */*loader*/, Document doc, Status status)
{
    // Note that Loader::~Loader() is private, so you cannot delete Loader instances.
    // You don't need to do that anyway since Loader instances delete themselves.

    if (status != Success)
    {
        faviconChanged(xmlUrl, KGlobal::iconLoader()->loadIcon("cancel", KIcon::Small));
        m_fetchError=true;

        return;
    }

    m_document=doc;
    kdDebug() << "Feed fetched successfully [" << m_document.title() << "]" << endl;

    if (m_document.image())
    {
        connect (m_document.image(), SIGNAL(gotPixmap(const QPixmap &)),
                               this, SLOT(imageChanged(const QPixmap &)));
        m_document.image()->getPixmap();
    }

    if (updateTitle || title().isEmpty()) setTitle( m_document.title() );
    description = m_document.description();
    htmlUrl = m_document.link().url();

    Article::List::ConstIterator it;
    Article::List::ConstIterator en = m_document.articles().end();
    for (it = m_document.articles().begin(); it != en; ++it)
    {
        articles.append( MyArticle( (*it) ) );
    }

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
        item()->setPixmap(0, p);
        favicon=p;
        emit(faviconLoaded(this)); // emit so that other sources can be updated.. not used right now
    }
}

void Feed::imageChanged(const QPixmap &p)
{
    image=p;
    // TODO check if present in data dir
    image.save(KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+title()+".png","PNG");
    emit(imageLoaded(this));
}

// = ArticleSequence ===================================================== //

struct ArticleSequence::Private
{
   int dummy;
//   bool doSort :1;
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

/*
void ArticleSequence::doNotSort()
{
    d->doSort = false;
}

void ArticleSequence::sort()
{
    d->doSort = true;
//    qHeapSort( *this );
}
*/

#include "feed.moc"
