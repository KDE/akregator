/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "akregatorconfig.h"
#include "archive.h"
#include "feed.h"
#include "feedgroup.h"
#include "fetchtransaction.h"

#include <kurl.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>

#include <qtimer.h>
#include <qdatetime.h>
#include <qlistview.h>
#include <qdom.h>

using namespace Akregator;
using namespace RSS;

QString Feed::archiveModeToString(ArchiveMode mode)
{
    switch (mode)
    {
        case keepAllArticles:
            return "keepAllArticles";
        case disableArchiving:
            return "disableArchiving";
        case limitArticleNumber:
            return "limitArticleNumber";
        case limitArticleAge:
            return "limitArticleAge";
        default:    
            return "globalDefault";
   }

   // in a perfect world, this is never reached
   
   return "globalDefault";
}

Feed* Feed::fromOPML(QDomElement e)
{
    
    Feed* feed = 0;
    
    if( e.hasAttribute("xmlUrl") || e.hasAttribute("xmlurl") ) 
    {
        QString title = e.hasAttribute("text") ? e.attribute("text") : e.attribute("title");
        
        QString xmlUrl = e.hasAttribute("xmlUrl") ? e.attribute("xmlUrl") : e.attribute("xmlurl");

        bool useCustomFetchInterval = e.attribute("y") == "true" ? true : false;
        
        QString htmlUrl = e.attribute("htmlUrl");
        QString description = e.attribute("description");
        int fetchInterval = e.attribute("fetchInterval").toUInt();
        ArchiveMode archiveMode = stringToArchiveMode(e.attribute("archiveMode"));
        int maxArticleAge = e.attribute("maxArticleAge").toUInt();
        int maxArticleNumber = e.attribute("maxArticleNumber").toUInt();
      
        feed = new Feed();
        feed->setTitle(title);
        feed->setXmlUrl(xmlUrl);
        feed->setCustomFetchIntervalEnabled(useCustomFetchInterval);
        feed->setHtmlUrl(htmlUrl);
        feed->setDescription(description);
        feed->setArchiveMode(archiveMode);
        feed->setFetchInterval(fetchInterval);
        feed->setMaxArticleAge(maxArticleAge);
        feed->setMaxArticleNumber(maxArticleNumber);  
    }   
    
    return feed;
}            

ArticleSequence Feed::articles()
{ 
    return m_articles; 
}

Feed::ArchiveMode Feed::stringToArchiveMode(const QString& str)
{
    if (str == "globalDefault")
        return globalDefault;
    if (str == "keepAllArticles")
        return keepAllArticles;          
    if (str == "disableArchiving")
        return disableArchiving;
    if (str == "limitArticleNumber")
        return limitArticleNumber;
    if (str == "limitArticleAge")
        return limitArticleAge;
        
    return globalDefault;    
}


Feed::Feed()
    : TreeNode()
        , m_archiveMode(globalDefault) 
        , m_maxArticleAge(60) 
        , m_maxArticleNumber(1000) 
        , m_transaction(0)
        , m_fetchError(false)
        , m_fetchTries(0)
        , m_loader(0)
        , m_merged(false)
        , m_unread(0)
        , m_articles()
{
}

Feed::~Feed()
{}

QDomElement Feed::toOPML( QDomElement parent, QDomDocument document ) const
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    el.setAttribute( "title", title() );
    el.setAttribute( "xmlUrl", m_xmlUrl );
    el.setAttribute( "htmlUrl", m_htmlUrl );
    el.setAttribute( "description", m_description );
    el.setAttribute( "autoFetch", (useCustomFetchInterval() ? "true" : "false") );
    el.setAttribute( "fetchInterval", QString::number(fetchInterval()) );
    el.setAttribute( "archiveMode", archiveModeToString(m_archiveMode) );
    el.setAttribute( "maxArticleAge", m_maxArticleAge );
    el.setAttribute( "maxArticleNumber", m_maxArticleNumber );
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

    if (!m_htmlUrl.isEmpty())
    {
        QDomElement lnode = doc.createElement( "link" );
        QDomText ht=doc.createTextNode( m_htmlUrl );
        lnode.appendChild(ht);
        channode.appendChild(lnode);
    }

    // rss 2.0 requires channel description
    QDomElement dnode = doc.createElement( "description" );
    QDomText dt=doc.createTextNode( m_htmlUrl );
    dnode.appendChild(dt);
    channode.appendChild(dnode);

    
    // get archive size limit, -1 if unlimited
    
    int limit = -1;
    if (m_archiveMode == disableArchiving)
        limit = 0;
    else
    if (m_archiveMode == limitArticleNumber)
        limit = m_maxArticleNumber;
    else if (m_archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleNumber)
        limit = Settings::maxArticleNumber();
    else if (m_archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::disableArchiving)
         limit = 0;
        
    ArticleSequence::ConstIterator it;
    ArticleSequence::ConstIterator en=m_articles.end();

    // if a limit exists, only dump "limit" number of articles plus articles with keep flag set
      
    if (limit != -1)
    {
        int count = 0; 
        for (it = m_articles.begin(); it != en; ++it)
            if ( count < limit || (*it).keep() )
            {
                 QDomElement enode = doc.createElement( "item" );
                 (*it).dumpXmlData(enode, doc);
                 channode.appendChild(enode);
                 if ( !(*it).keep() )
                    ++count;
            }
    }
    else // save everything, like we did before
        for (it = m_articles.begin(); it != en; ++it)
        {
            QDomElement enode = doc.createElement( "item" );
            (*it).dumpXmlData(enode, doc);
            channode.appendChild(enode);
        }
}

void Feed::slotMarkAllArticlesAsRead()
{
    if (m_unread > 0)
    {
        ArticleSequence::Iterator it;
        ArticleSequence::Iterator en = m_articles.end();
        
        for (it = m_articles.begin(); it != en; ++it)
            (*it).setStatus(MyArticle::Read);
                
        m_unread = 0;
        Archive::save(this);
        modified();
    }
}
void Feed::slotAddToFetchTransaction(FetchTransaction* transaction)
{
    transaction->addFeed(this);
}


void Feed::appendArticles(const Document &d)
{
    bool changed = false;
    
    m_articles.enableSorting(false);
    Article::List d_articles = d.articles();
    Article::List::ConstIterator it;
    Article::List::ConstIterator en = d_articles.end();
    
    int nudge=0;

    for (it = d_articles.begin(); it != en; ++it)
    {
        MyArticle mya(*it);

        // if archive isn't loaded, append. Otherwise check for dupes.
        if (!m_merged || m_articles.find(mya) == m_articles.end() )
        {
            if ( m_merged )
                mya.setStatus(MyArticle::New);
            else
            {
                if (mya.status() == MyArticle::New)
                    mya.setStatus(MyArticle::Unread);
            }

            mya.offsetFetchTime(nudge);
            appendArticle(mya);
            changed = true;
            nudge--;
        }
    }
    
    m_articles.enableSorting(true);
    m_articles.sort();
    if (changed)
        modified();
}

bool Feed::usesExpiryByAge() const
{
    return ( m_archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleAge) || m_archiveMode == limitArticleAge;
}

bool Feed::isExpired(const MyArticle& a) const
{
    QDateTime now = QDateTime::currentDateTime();
    int expiryAge = -1;
// check whether the feed uses the global default and the default is limitArticleAge
    if ( m_archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleAge)
        expiryAge = Settings::maxArticleAge() *24*3600;
    else // otherwise check if this feed has limitArticleAge set
        if ( m_archiveMode == limitArticleAge)
            expiryAge = m_maxArticleAge *24*3600;

    return ( expiryAge != -1 && a.pubDate().secsTo(now) > expiryAge);
    

}

void Feed::appendArticle(const MyArticle& a)
{
    
    if ( a.keep() || ( !usesExpiryByAge() || !isExpired(a) ) ) // if not expired
    {
        if (a.status() != MyArticle::Read)
            m_unread++;
            
        ArticleSequence::Iterator it;
        ArticleSequence::Iterator end = m_articles.end();
        bool inserted = false;    
        it = m_articles.begin();
        
        while ( !inserted && it != end )
            if ( a >= (*it) )
                ++it;       
            else
                inserted = true;
        MyArticle a2(a);    
        a2.setFeed(this);    
        if ( inserted )
            m_articles.insert(it, a2);
        else
            m_articles.append(a2);    
    }    
}


void Feed::fetch(bool followDiscovery, FetchTransaction *trans)
{
    m_followDiscovery = followDiscovery;
    m_transaction = trans;
    m_fetchTries = 0;

    // mark all new as unread
    ArticleSequence::Iterator it;
    ArticleSequence::Iterator en = m_articles.end();
    for (it = m_articles.begin(); it != en; ++it)
    {
        if ((*it).status() == MyArticle::New)
        {
            (*it).setStatus(MyArticle::Unread);
        }
    }

    emit fetchStarted(this);

    tryFetch();
}

void Feed::abortFetch()
{
    if (m_loader)
    {
        m_loader->abort();
        emit fetchAborted(this);
    }
}

void Feed::tryFetch()
{
    m_fetchError = false;

    m_loader = Loader::create( this, SLOT(fetchCompleted(Loader *, Document, Status)) );
    m_loader->loadFrom( m_xmlUrl, new FileRetriever );
}

void Feed::fetchCompleted(Loader *l, Document doc, Status status)
{
    // Note that Loader::~Loader() is private, so you cannot delete Loader instances.
    // You don't need to do that anyway since Loader instances delete themselves.

    if (status!= Success)
    {
        if (m_followDiscovery && (status == ParseError) && (m_fetchTries < 3) && 			(l->discoveredFeedURL().isValid()))
        {
            m_fetchTries++;
            m_xmlUrl = l->discoveredFeedURL().url();
            emit fetchDiscovery(this);
            tryFetch();
            return;
        }
        else
        {
            m_fetchError = true;
            emit fetchError(this);
            return;
        }
    }

    // Restore favicon.
    if (m_favicon.isNull())
        loadFavicon();
//    else
  //      item()->setPixmap(0, m_favicon);

    m_fetchError=false;
    m_document=doc;
    //kdDebug() << "Feed fetched successfully [" << m_document.title() << "]" << endl;


    if (m_image.isNull())
    {
        QString u = m_xmlUrl;
        QString imageFileName = KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png";
        m_image=QPixmap(imageFileName, "PNG");

        // if we aint got teh image
        // and the feed provides one, get it....
        if (m_image.isNull() && m_document.image() && m_transaction)
            m_transaction->loadImage(this, m_document.image());
    }
    
    if ( title().isEmpty() ) 
        setTitle( KCharsets::resolveEntities(KCharsets::resolveEntities(m_document.title())) );

    Archive::load(this); // make sure archive is merged 
    
    m_description = m_document.description();
    m_htmlUrl = m_document.link().url();
    
    //kdDebug() << "ismerged reprots:::"<<isMerged()<<endl;

    appendArticles(m_document);

    m_loader=0;
    m_transaction=0;
    emit fetched(this);
}

void Feed::loadFavicon()
{
    if (!m_transaction)
	   return;
    m_transaction->loadIcon(this);
}

void Feed::slotDeleteExpiredArticles()
{
    bool changed = false;
    if ( !usesExpiryByAge() )
        return;

    ArticleSequence::ConstIterator it = m_articles.end();
    ArticleSequence::ConstIterator tmp;
    ArticleSequence::ConstIterator begin = m_articles.begin();
    // when we found an article which is not yet expired, we can stop, since articles are sorted by date
    bool foundNotYetExpired = false;

    while ( !foundNotYetExpired && it != begin )
    {
        --it;
        if (!(*it).keep())
        {
            if ( isExpired(*it) )
            {
                tmp = it;
                m_articles.remove(*tmp);
                changed = true;
            }
            else
                foundNotYetExpired = true;
        }
    }
    if (changed)
        modified();
}

void Feed::setFavicon(const QPixmap &p)
{
    if (p.isNull())
	   return;
    m_favicon = p;
 //   if (!m_fetchError && item())
 //           item()->setPixmap(0, p);
    modified();
}

void Feed::setImage(const QPixmap &p)
{
    if (p.isNull())
        return;
    m_image=p;
    QString u = m_xmlUrl;
    m_image.save(KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png","PNG");
    emit(imageLoaded(this));
}

Feed::ArchiveMode Feed::archiveMode() const
{
    return m_archiveMode;
}

void Feed::setArchiveMode(ArchiveMode archiveMode)
{
    m_archiveMode = archiveMode;
}

void Feed::setUnread(int unread)
{ 
    if (unread != m_unread)
    {    
        m_unread = unread;
        modified();
    }    
}
int Feed::totalCount() const
{
    return m_articles.count();
}

TreeNode* Feed::next()
{
    if ( nextSibling() )
        return nextSibling();
    
    FeedGroup* p = parent();
    while (p)
    {
        if ( p->nextSibling() )
            return p->nextSibling();
        else
            p = p->parent();
    }
    return 0;
}

#include "feed.moc"
