/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#include <qtimer.h>
#include <qdatetime.h>
#include <qlistview.h>
#include <qdom.h>
#include <qpixmap.h>

#include <kurl.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "articlelist.h"
#include "akregatorconfig.h"
#include "feed.h"
#include "feedgroup.h"
#include "fetchqueue.h"
#include "feediconmanager.h"
#include "feedstorage.h"
#include "articleinterceptor.h"
#include "storage.h"

#include "librss/librss.h"

namespace Akregator {

class Feed::FeedPrivate
{
    public:
        bool autoFetch;
        int fetchInterval;
        ArchiveMode archiveMode;
        int maxArticleAge;
        int maxArticleNumber;
        bool markImmediatelyAsRead;
        bool useNotification;
        bool loadLinkedWebsite;
        int lastFetched;

        bool fetchError;
        int fetchTries;
        bool followDiscovery;
        RSS::Loader* loader;
        bool articlesLoaded;
        Backend::FeedStorage* archive;

        QString xmlUrl;
        QString htmlUrl;
        QString description;

        /** list of feed articles */
        ArticleList articles;

        /** caches guids of new articles for notication */
        QStringList newArticles;

        /** list of deleted articles. This contains **/
        ArticleList deletedArticles;
        
        /** caches guids of deleted articles for notification */
        QStringList deletedArticlesNotify;
        
        QPixmap imagePixmap;
        RSS::Image image;
        QPixmap favicon;
};
            
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

        bool useCustomFetchInterval = e.attribute("useCustomFetchInterval") == "true";

        QString htmlUrl = e.attribute("htmlUrl");
        QString description = e.attribute("description");
        int fetchInterval = e.attribute("fetchInterval").toUInt();
        ArchiveMode archiveMode = stringToArchiveMode(e.attribute("archiveMode"));
        int maxArticleAge = e.attribute("maxArticleAge").toUInt();
        int maxArticleNumber = e.attribute("maxArticleNumber").toUInt();
        bool markImmediatelyAsRead = e.attribute("markImmediatelyAsRead") == "true";
        bool useNotification = e.attribute("useNotification") == "true";
        bool loadLinkedWebsite = e.attribute("loadLinkedWebsite") == "true";
        uint id = e.attribute("id").toUInt();

        feed = new Feed();
        feed->setTitle(title);
        feed->setXmlUrl(xmlUrl);
        feed->setCustomFetchIntervalEnabled(useCustomFetchInterval);
        feed->setHtmlUrl(htmlUrl);
        feed->setId(id);
        feed->setDescription(description);
        feed->setArchiveMode(archiveMode);
        feed->setUseNotification(useNotification);
        feed->setFetchInterval(fetchInterval);
        feed->setMaxArticleAge(maxArticleAge);
        feed->setMaxArticleNumber(maxArticleNumber);
        feed->setMarkImmediatelyAsRead(markImmediatelyAsRead);
        feed->setLoadLinkedWebsite(loadLinkedWebsite);
        feed->loadArticles(); // TODO: make me fly: make this delayed
        
    }

    return feed;
}

ArticleList Feed::articles()
{
    if (!d->articlesLoaded)
        loadArticles();
    return d->articles;
}

void Feed::loadArticles()
{
    if (d->articlesLoaded)
        return;

    if (!d->archive)
        d->archive = Backend::Storage::getInstance()->archiveFor(xmlUrl());

    d->articles.enableSorting(false);
    QStringList list = d->archive->articles();
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it)
    {
        Article mya(*it, this);
        d->articles.append(mya);
        if (mya.isDeleted())
            d->deletedArticles.append(mya);
    }
    d->articles.enableSorting(true);
    d->articles.sort();
    d->articlesLoaded = true;
    enforceLimitArticleNumber();
    recalcUnreadCount();
}

void Feed::recalcUnreadCount()
{
    ArticleList tarticles = articles();
    ArticleList::Iterator it;
    ArticleList::Iterator en = tarticles.end();

    int oldUnread = d->archive->unread();
    
    int unread = 0;

    for (it = tarticles.begin(); it != en; ++it)
        if (!(*it).isDeleted() && (*it).status() != Article::Read)
            ++unread;

    if (unread != oldUnread)
    {
        d->archive->setUnread(unread);
        modified();
    }
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

Feed::Feed() : TreeNode(), d(new FeedPrivate)
{
    d->autoFetch = false;
    d->fetchInterval = 30;
    d->archiveMode = globalDefault;
    d->maxArticleAge = 60;
    d->maxArticleNumber = 1000;
    d->markImmediatelyAsRead = false;
    d->useNotification = false;
    d->fetchError = false;
    d->fetchTries = 0;
    d->loader = 0;
    d->articlesLoaded = false;
    d->archive = 0;
    d->loadLinkedWebsite = false;
}

Feed::~Feed()
{
    slotAbortFetch();
    // tell the world that this node is destroyed
    emit signalDestroyed(this);
    delete d;
    d = 0;
}

bool Feed::useCustomFetchInterval() const { return d->autoFetch; }

void Feed::setCustomFetchIntervalEnabled(bool enabled) { d->autoFetch = enabled; }

int Feed::fetchInterval() const { return d->fetchInterval; }

void Feed::setFetchInterval(int interval) { d->fetchInterval = interval; }

int Feed::maxArticleAge() const { return d->maxArticleAge; }

void Feed::setMaxArticleAge(int maxArticleAge) { d->maxArticleAge = maxArticleAge; }

int Feed::maxArticleNumber() const { return d->maxArticleNumber; }

void Feed::setMaxArticleNumber(int maxArticleNumber) { d->maxArticleNumber = maxArticleNumber; }

bool Feed::markImmediatelyAsRead() const { return d->markImmediatelyAsRead; }

void Feed::setMarkImmediatelyAsRead(bool enabled)
{
    d->markImmediatelyAsRead = enabled;
    if (enabled)
        slotMarkAllArticlesAsRead();
}

void Feed::setUseNotification(bool enabled)
{
    d->useNotification = enabled;
}

bool Feed::useNotification() const
{
    return d->useNotification;
}

void Feed::setLoadLinkedWebsite(bool enabled)
{
    d->loadLinkedWebsite = enabled;
}

bool Feed::loadLinkedWebsite() const
{
    return d->loadLinkedWebsite;
}
            
const QPixmap& Feed::favicon() const { return d->favicon; }

const QPixmap& Feed::image() const { return d->imagePixmap; }

const QString& Feed::xmlUrl() const { return d->xmlUrl; }

void Feed::setXmlUrl(const QString& s) { d->xmlUrl = s; }

const QString& Feed::htmlUrl() const { return d->htmlUrl; }

void Feed::setHtmlUrl(const QString& s) { d->htmlUrl = s; }

const QString& Feed::description() const { return d->description; }

void Feed::setDescription(const QString& s) { d->description = s; }

bool Feed::fetchErrorOccurred() { return d->fetchError; }

bool Feed::isArticlesLoaded() const { return d->articlesLoaded; }


QDomElement Feed::toOPML( QDomElement parent, QDomDocument document ) const
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    el.setAttribute( "title", title() );
    el.setAttribute( "xmlUrl", d->xmlUrl );
    el.setAttribute( "htmlUrl", d->htmlUrl );
    el.setAttribute( "id", QString::number(id()) );
    el.setAttribute( "description", d->description );
    el.setAttribute( "useCustomFetchInterval", (useCustomFetchInterval() ? "true" : "false") );
    el.setAttribute( "fetchInterval", QString::number(fetchInterval()) );
    el.setAttribute( "archiveMode", archiveModeToString(d->archiveMode) );
    el.setAttribute( "maxArticleAge", d->maxArticleAge );
    el.setAttribute( "maxArticleNumber", d->maxArticleNumber );
    if (d->markImmediatelyAsRead)
        el.setAttribute( "markImmediatelyAsRead", "true" );
    if (d->useNotification)
        el.setAttribute( "useNotification", "true" );
    if (d->loadLinkedWebsite)
        el.setAttribute( "loadLinkedWebsite", "true" );
    el.setAttribute( "maxArticleNumber", d->maxArticleNumber );
    el.setAttribute( "type", "rss" ); // despite some additional fields, its still "rss" OPML
    el.setAttribute( "version", "RSS" );
    parent.appendChild( el );
    return el;
}

void Feed::slotMarkAllArticlesAsRead()
{
    if (unread() > 0)
    {
        setNotificationMode(false, true);
        ArticleList tarticles = articles();
        ArticleList::Iterator it;
        ArticleList::Iterator en = tarticles.end();

        for (it = tarticles.begin(); it != en; ++it)
            (*it).setStatus(Article::Read);
        setNotificationMode(true, true);
    }
}
void Feed::slotAddToFetchQueue(FetchQueue* queue)
{
    queue->addFeed(this);
}


void Feed::appendArticles(const RSS::Document &doc)
{
    bool changed = false;

    d->articles.enableSorting(false);
    RSS::Article::List d_articles = doc.articles();
    RSS::Article::List::ConstIterator it;
    RSS::Article::List::ConstIterator en = d_articles.end();

    int nudge=0;
    
    ArticleList deletedArticles = d->deletedArticles;
    
    for (it = d_articles.begin(); it != en; ++it)
    {
        Article mya(*it, this);

        ArticleList::Iterator old = d->articles.find(mya);

        if ( old == d->articles.end() ) // article not in list
        {
            mya.offsetPubDate(nudge);
            nudge--;
            appendArticle(mya);

            QValueList<ArticleInterceptor*> interceptors = ArticleInterceptorManager::self()->interceptors();
            for (QValueList<ArticleInterceptor*>::ConstIterator it = interceptors.begin(); it != interceptors.end(); ++it)
                (*it)->processArticle(mya);
            
            d->newArticles.append(mya.guid());
            
            if (!mya.isDeleted() && !markImmediatelyAsRead())
                mya.setStatus(Article::New);
            else
                mya.setStatus(Article::Read);
                
            changed = true;
        }
        // if the article's guid is no hash but an ID, we have to check if the article was updated. That's done by comparing the hash values.
        else if (!mya.guidIsHash() && mya.hash() != (*old).hash() && !(*old).isDeleted())
        {
            mya.setKeep((*old).keep());
            (*old).setStatus(Article::Read);
            d->articles.remove(old);
            appendArticle(mya);
            // reset status to New
            if (!mya.isDeleted() && !markImmediatelyAsRead())
                mya.setStatus(Article::New);
            changed = true;
        }
        else if ((*old).isDeleted())
            deletedArticles.remove(mya);
    }
    
    ArticleList::ConstIterator dit = deletedArticles.begin();
    ArticleList::ConstIterator dtmp;
    ArticleList::ConstIterator den = deletedArticles.end();

    // delete articles with delete flag set completely from archive, which aren't in the current feed source anymore
    while (dit != den)
    {
        dtmp = dit;
        ++dit;
        d->articles.remove(*dtmp);
        d->archive->deleteArticle((*dtmp).guid());
        d->deletedArticles.remove(*dtmp);
    }
    
    d->articles.enableSorting(true);
    d->articles.sort();
    if (changed)
        modified();
}

bool Feed::usesExpiryByAge() const
{
    return ( d->archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleAge) || d->archiveMode == limitArticleAge;
}

bool Feed::isExpired(const Article& a) const
{
    QDateTime now = QDateTime::currentDateTime();
    int expiryAge = -1;
// check whether the feed uses the global default and the default is limitArticleAge
    if ( d->archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleAge)
        expiryAge = Settings::maxArticleAge() *24*3600;
    else // otherwise check if this feed has limitArticleAge set
        if ( d->archiveMode == limitArticleAge)
            expiryAge = d->maxArticleAge *24*3600;

    return ( expiryAge != -1 && a.pubDate().secsTo(now) > expiryAge);
}

void Feed::appendArticle(const Article& a)
{
    if ( a.keep() || ( !usesExpiryByAge() || !isExpired(a) ) ) // if not expired
    {
        if (!d->articles.contains(a))
        {
            d->articles.append(a);
            if (!a.isDeleted() && a.status() != Article::Read)
                setUnread(unread()+1);
        }
    }
}


void Feed::fetch(bool followDiscovery)
{
    d->followDiscovery = followDiscovery;
    d->fetchTries = 0;

    // mark all new as unread
    ArticleList::Iterator it;
    ArticleList::Iterator en = d->articles.end();
    for (it = d->articles.begin(); it != en; ++it)
    {
        if ((*it).status() == Article::New)
        {
            (*it).setStatus(Article::Unread);
        }
    }

    emit fetchStarted(this);

    tryFetch();
}

void Feed::slotAbortFetch()
{
    if (d->loader)
    {
        d->loader->abort();
    }
}

void Feed::tryFetch()
{
    d->fetchError = false;

    d->loader = RSS::Loader::create( this, SLOT(fetchCompleted(Loader *, Document, Status)) );
    //connect(d->loader, SIGNAL(progress(unsigned long)), this, SLOT(slotSetProgress(unsigned long)));
    d->loader->loadFrom( d->xmlUrl, new RSS::FileRetriever );
}

void Feed::slotImageFetched(const QPixmap& image)
{
    setImage(image);
}

void Feed::fetchCompleted(RSS::Loader *l, RSS::Document doc, RSS::Status status)
{
    // Note that loader instances delete themselves
    d->loader = 0;

    // fetching wasn't successful:
    if (status != RSS::Success)
    {
        if (status == RSS::Aborted)
        {
            d->fetchError = false;
            emit fetchAborted(this);
        }
        else if (d->followDiscovery && (status == RSS::ParseError) && (d->fetchTries < 3) && (l->discoveredFeedURL().isValid()))
        {
            d->fetchTries++;
            d->xmlUrl = l->discoveredFeedURL().url();
            emit fetchDiscovery(this);
            tryFetch();
        }
        else
        {
            d->fetchError = true;
            emit fetchError(this);
        }
        return;
    }

    loadArticles(); // TODO: make me fly: make this delayed
    
    // Restore favicon.
    if (d->favicon.isNull())
        loadFavicon();

    d->fetchError = false;
    
    if (d->imagePixmap.isNull())
    {
        QString u = d->xmlUrl;
        QString imageFileName = KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png";
        d->imagePixmap=QPixmap(imageFileName, "PNG");

        // if we aint got teh image and the feed provides one, get it....
        if (d->imagePixmap.isNull() && doc.image())
        {
            d->image = *doc.image();
            connect(&d->image, SIGNAL(gotPixmap(const QPixmap&)), this, SLOT(slotImageFetched(const QPixmap&)));
            d->image.getPixmap();
        }   
    }

    if (title().isEmpty())
        setTitle( KCharsets::resolveEntities(KCharsets::resolveEntities(doc.title())) );

    d->description = doc.description();
    d->htmlUrl = doc.link().url();

    appendArticles(doc);

    d->archive->setLastFetch( QDateTime::currentDateTime().toTime_t());
    emit fetched(this);
}

void Feed::loadFavicon()
{
    FeedIconManager::self()->fetchIcon(this);
}

void Feed::slotDeleteExpiredArticles()
{
    if ( !usesExpiryByAge() )
        return;

    ArticleList::Iterator it = d->articles.end();
    ArticleList::Iterator tmp;
    ArticleList::Iterator begin = d->articles.begin();
    // when we found an article which is not yet expired, we can stop, since articles are sorted by date
    bool foundNotYetExpired = false;

    setNotificationMode(false);
        
    while ( !foundNotYetExpired && it != begin )
    {
        --it;
        if (!(*it).keep())
        {
            if ( isExpired(*it) )
            {
                tmp = it;
                (*tmp).setDeleted();
            }
            else
                foundNotYetExpired = true;
        }
    }
    setNotificationMode(true);
}

void Feed::setFavicon(const QPixmap &p)
{
    d->favicon = p;
    modified();
}

void Feed::setImage(const QPixmap &p)
{
    if (p.isNull())
        return;
    d->imagePixmap=p;
    QString u = d->xmlUrl;
    d->imagePixmap.save(KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png","PNG");
    modified();
}

Feed::ArchiveMode Feed::archiveMode() const
{
    return d->archiveMode;
}

void Feed::setArchiveMode(ArchiveMode archiveMode)
{
    d->archiveMode = archiveMode;
}

int Feed::unread() const
{
    return d->archive ? d->archive->unread() : 0;
}

void Feed::setUnread(int unread)
{
    if (d->archive && unread != d->archive->unread())
    {
        d->archive->setUnread(unread);
        modified();
    }
}

void Feed::slotArticleStatusChanged(int oldStatus, const Article& article)
{
    if (oldStatus == Article::Read && article.status() != Article::Read)
        setUnread(unread()+1);
    else if (oldStatus != Article::Read &&  article.status() == Article::Read)
        setUnread(unread()-1);
}

void Feed::slotArticleDeleted(const Article& mya)
{
    if (!d->deletedArticles.contains(mya))
        d->deletedArticles.append(mya);
    d->deletedArticlesNotify.append(mya.guid());
    modified();
}

int Feed::totalCount() const
{
    return d->articles.count();
}

TreeNode* Feed::next()
{
    if ( nextSibling() )
        return nextSibling();

    Folder* p = parent();
    while (p)
    {
        if ( p->nextSibling() )
            return p->nextSibling();
        else
            p = p->parent();
    }
    return 0;
}

void Feed::modified()
{
    if (!d->newArticles.isEmpty())
    {
        emit signalArticlesAdded(id(), d->newArticles);
        d->newArticles.clear();
    }
    if (!d->deletedArticlesNotify.isEmpty())
    {
        emit signalArticlesDeleted(id(), d->deletedArticlesNotify);
        d->deletedArticlesNotify.clear();
    }
    
    TreeNode::modified();
}

void Feed::enforceLimitArticleNumber()
{
    int limit = -1;
    if (d->archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleNumber)
        limit = Settings::maxArticleNumber();
    else if (d->archiveMode == limitArticleNumber)
        limit = maxArticleNumber();
        
    if (limit == -1 || limit >= d->articles.count() - d->deletedArticles.count())
        return;

    setNotificationMode(false);
    ArticleList::Iterator it = d->articles.begin();
    ArticleList::Iterator tmp;
    ArticleList::Iterator en = d->articles.end();

    int c = 0;
    while (it != en)
    {
        tmp = it;
        ++it;
        if (c < limit)
        {
            if (!(*tmp).isDeleted() && !(*tmp).keep())
               c++;
        }
        else if (!(*tmp).keep())
            (*tmp).setDeleted();
    }
    setNotificationMode(true);
}

} // namespace Akregator
#include "feed.moc"
