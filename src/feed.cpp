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
#include <qmap.h>
#include <qpixmap.h>
#include <qvaluelist.h>

#include <kurl.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include "akregatorconfig.h"
#include "article.h"
#include "articleinterceptor.h"
#include "feed.h"
#include "folder.h"
#include "fetchqueue.h"
#include "feediconmanager.h"
#include "feedstorage.h"
#include "storage.h"
#include "treenodevisitor.h"

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
        QMap<QString, Article> articles;

        /** caches guids of tagged articles. key: tag, value: list of guids */
        QMap<QString, QStringList> taggedArticles;

        /** list of deleted articles. This contains **/
        QValueList<Article> deletedArticles;
        
        /** caches guids of deleted articles for notification */
   
        QValueList<Article> addedArticlesNotify;
        QValueList<Article> removedArticlesNotify;
        QValueList<Article> updatedArticlesNotify;
        
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

bool Feed::accept(TreeNodeVisitor* visitor)
{
    if (visitor->visitFeed(this))
        return true;
    else
        return visitor->visitTreeNode(this);
}

QStringList Feed::tags() const
{
    return d->archive->tags();
}

QValueList<Article> Feed::articles(const QString& tag)
{
    if (!d->articlesLoaded)
        loadArticles();
    if (tag.isNull())
        return d->articles.values();
    else
    {
        // TODO, FIXME: make this sane!
        QValueList<Article> tagged;
        QStringList guids = d->archive->articles(tag);
        for (QStringList::ConstIterator it = guids.begin(); it != guids.end(); ++it)
            tagged += d->articles[*it];
        return tagged;
        
    }
}

void Feed::loadArticles()
{
    if (d->articlesLoaded)
        return;

    if (!d->archive)
        d->archive = Backend::Storage::getInstance()->archiveFor(xmlUrl());

    QStringList list = d->archive->articles();
    for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        Article mya(*it, this);
        d->articles[mya.guid()] = mya;
        if (mya.isDeleted())
            d->deletedArticles.append(mya);
    }
    
    d->articlesLoaded = true;
    enforceLimitArticleNumber();
    recalcUnreadCount();
}

void Feed::recalcUnreadCount()
{
    QValueList<Article> tarticles = articles();
    QValueList<Article>::Iterator it;
    QValueList<Article>::Iterator en = tarticles.end();

    int oldUnread = d->archive->unread();
    
    int unread = 0;

    for (it = tarticles.begin(); it != en; ++it)
        if (!(*it).isDeleted() && (*it).status() != Article::Read)
            ++unread;

    if (unread != oldUnread)
    {
        d->archive->setUnread(unread);
        nodeModified();
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
        QValueList<Article> tarticles = articles();
        QValueList<Article>::Iterator it;
        QValueList<Article>::Iterator en = tarticles.end();

        for (it = tarticles.begin(); it != en; ++it)
            (*it).setStatus(Article::Read);
        setNotificationMode(true, true);
    }
}
void Feed::slotAddToFetchQueue(FetchQueue* queue, bool intervalFetchOnly)
{
    if (!intervalFetchOnly)
        queue->addFeed(this);
    else
    {
        int interval = -1;

        if (useCustomFetchInterval() )
            interval = fetchInterval() * 60;
        else
            if ( Settings::useIntervalFetch() )
                interval = Settings::autoFetchInterval() * 60;

        uint lastFetch = d->archive->lastFetch();

        uint now = QDateTime::currentDateTime().toTime_t();

        if ( interval > 0 && now - lastFetch >= (uint)interval )
            queue->addFeed(this);
    }
}


void Feed::appendArticles(const RSS::Document &doc)
{
    bool changed = false;

    RSS::Article::List d_articles = doc.articles();
    RSS::Article::List::ConstIterator it;
    RSS::Article::List::ConstIterator en = d_articles.end();

    int nudge=0;
    
    QValueList<Article> deletedArticles = d->deletedArticles;

    for (it = d_articles.begin(); it != en; ++it)
    {
        if ( !d->articles.contains((*it).guid()) ) // article not in list
        {
            Article mya(*it, this);
            mya.offsetPubDate(nudge);
            nudge--;
            appendArticle(mya);

            QValueList<ArticleInterceptor*> interceptors = ArticleInterceptorManager::self()->interceptors();
            for (QValueList<ArticleInterceptor*>::ConstIterator it = interceptors.begin(); it != interceptors.end(); ++it)
                (*it)->processArticle(mya);
            
            d->addedArticlesNotify.append(mya);
            
            if (!mya.isDeleted() && !markImmediatelyAsRead())
                mya.setStatus(Article::New);
            else
                mya.setStatus(Article::Read);
                
            changed = true;
        }
        else // article is in list
        {
            // if the article's guid is no hash but an ID, we have to check if the article was updated. That's done by comparing the hash values.
            Article old = d->articles[(*it).guid()];
            Article mya(*it, this);          
            if (!mya.guidIsHash() && mya.hash() != old.hash() && !old.isDeleted())
            {
                mya.setKeep(old.keep());
                old.setStatus(Article::Read);
                d->articles.remove(old.guid());
                appendArticle(mya);
                // reset status to New
                if (!mya.isDeleted() && !markImmediatelyAsRead())
                    mya.setStatus(Article::New);
                d->updatedArticlesNotify.append(mya);
                changed = true;
            }
            else if (old.isDeleted())
                deletedArticles.remove(mya);
        }    
    }
    
    QValueList<Article>::ConstIterator dit = deletedArticles.begin();
    QValueList<Article>::ConstIterator dtmp;
    QValueList<Article>::ConstIterator den = deletedArticles.end();

    // delete articles with delete flag set completely from archive, which aren't in the current feed source anymore
    while (dit != den)
    {
        dtmp = dit;
        ++dit;
        d->articles.remove((*dtmp).guid());
        d->archive->deleteArticle((*dtmp).guid());
        d->deletedArticles.remove(*dtmp);
    }
    
    if (changed)
        articlesModified();
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
    if ( (a.keep() && Settings::doNotExpireImportantArticles()) || ( !usesExpiryByAge() || !isExpired(a) ) ) // if not expired
    {
        if (!d->articles.contains(a.guid()))
        {
            d->articles[a.guid()] = a;
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
    QValueList<Article> articles = d->articles.values();
    QValueList<Article>::Iterator it;
    QValueList<Article>::Iterator en = articles.end();
    for (it = articles.begin(); it != en; ++it)
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

    QValueList<Article> articles = d->articles.values();
    
    QValueList<Article>::Iterator en = articles.end();

    setNotificationMode(false);

    for (QValueList<Article>::Iterator it = articles.begin(); it != en; ++it)
    {
        if (!(*it).keep() && isExpired(*it))
        {
                (*it).setDeleted();
        }
    }
    setNotificationMode(true);
}

void Feed::setFavicon(const QPixmap &p)
{
    d->favicon = p;
    nodeModified();
}

void Feed::setImage(const QPixmap &p)
{
    if (p.isNull())
        return;
    d->imagePixmap=p;
    QString u = d->xmlUrl;
    d->imagePixmap.save(KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png","PNG");
    nodeModified();
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
        nodeModified();
    }
}


void Feed::setArticleDeleted(Article& a)
{
    if (!d->deletedArticles.contains(a))
        d->deletedArticles.append(a);
    d->removedArticlesNotify.append(a);
    articlesModified();
}

void Feed::setArticleChanged(Article& a, int oldStatus)
{
    if (oldStatus != -1)
    {
        int newStatus = a.status();
        if (oldStatus == Article::Read && newStatus != Article::Read)
            setUnread(unread()+1);
        else if (oldStatus != Article::Read && newStatus == Article::Read)
            setUnread(unread()-1);
    }
    d->updatedArticlesNotify.append(a);
    articlesModified();
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

void Feed::doArticleNotification()
{
    if (!d->addedArticlesNotify.isEmpty())
    {
        emit signalArticlesAdded(this, d->addedArticlesNotify);
        d->addedArticlesNotify.clear();
    }
    if (!d->updatedArticlesNotify.isEmpty())
    {
        emit signalArticlesUpdated(this, d->updatedArticlesNotify);
        d->updatedArticlesNotify.clear();
    }
    if (!d->removedArticlesNotify.isEmpty())
    {
        emit signalArticlesRemoved(this, d->removedArticlesNotify);
        d->removedArticlesNotify.clear();
    }
    TreeNode::doArticleNotification();
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
    QValueList<Article> articles = d->articles.values();
    qHeapSort(articles);
    QValueList<Article>::Iterator it = articles.begin();
    QValueList<Article>::Iterator tmp;
    QValueList<Article>::Iterator en = articles.end();

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
