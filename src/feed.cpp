/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "akregatorconfig.h"
#include "article.h"
#include "articlejobs.h"
#include "feed.h"
#include "feediconmanager.h"
#include "feedstorage.h"
#include "fetchqueue.h"
#include "folder.h"
#include "notificationmanager.h"
#include "storage.h"
#include "treenodevisitor.h"
#include "types.h"
#include "utils.h"
#include <syndication/syndication.h>

#include <kdebug.h>
#include <kglobal.h>
#include <KIcon>
#include <kstandarddirs.h>
#include <kurl.h>
#include <KRandom>

//#include <qtl.h>

#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QIcon>
#include <QList>
#include <QPixmap>
#include <QTimer>

#include <boost/bind.hpp>

#include <memory>

using Syndication::ItemPtr;
using namespace Akregator;
using namespace boost;

class Feed::Private
{
        Feed* const q;
    public:
        explicit Private( Backend::Storage* storage, Feed* qq );

        Backend::Storage* storage;
        bool autoFetch;
        int fetchInterval;
        ArchiveMode archiveMode;
        int maxArticleAge;
        int maxArticleNumber;
        bool markImmediatelyAsRead;
        bool useNotification;
        bool loadLinkedWebsite;
        int lastFetched;

        Syndication::ErrorCode fetchErrorCode;
        int fetchTries;
        bool followDiscovery;
        Syndication::Loader* loader;
        bool articlesLoaded;
        Backend::FeedStorage* archive;

        QString xmlUrl;
        QString htmlUrl;
        QString description;

        /** list of feed articles */
        QHash<QString, Article> articles;

        /** list of deleted articles. This contains **/
        QList<Article> deletedArticles;

        /** caches guids of deleted articles for notification */

        QList<Article> addedArticlesNotify;
        QList<Article> removedArticlesNotify;
        QList<Article> updatedArticlesNotify;

        QPixmap imagePixmap;
        Syndication::ImagePtr image;
        QIcon favicon;
        mutable int totalCount;
        void setTotalCountDirty() const { totalCount = -1; }
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

Feed* Feed::fromOPML(QDomElement e, Backend::Storage* storage )
{

    if( !e.hasAttribute("xmlUrl") && !e.hasAttribute("xmlurl") && !e.hasAttribute("xmlURL") )
        return 0;

    QString title = e.hasAttribute("text") ? e.attribute("text") : e.attribute("title");

    QString xmlUrl = e.hasAttribute("xmlUrl") ? e.attribute("xmlUrl") : e.attribute("xmlurl");
    if (xmlUrl.isEmpty())
        xmlUrl = e.attribute("xmlURL");

    bool useCustomFetchInterval = e.attribute("useCustomFetchInterval") == "true";

    QString htmlUrl = e.attribute("htmlUrl");
    QString description = e.attribute("description");
    int fetchInterval = e.attribute("fetchInterval").toInt();
    ArchiveMode archiveMode = stringToArchiveMode(e.attribute("archiveMode"));
    int maxArticleAge = e.attribute("maxArticleAge").toUInt();
    int maxArticleNumber = e.attribute("maxArticleNumber").toUInt();
    bool markImmediatelyAsRead = e.attribute("markImmediatelyAsRead") == "true";
    bool useNotification = e.attribute("useNotification") == "true";
    bool loadLinkedWebsite = e.attribute("loadLinkedWebsite") == "true";
    uint id = e.attribute("id").toUInt();

    Feed* const feed = new Feed( storage );
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

    return feed;
}

bool Feed::accept(TreeNodeVisitor* visitor)
{
    if (visitor->visitFeed(this))
        return true;
    else
        return visitor->visitTreeNode(this);
}

QVector<const Folder*> Feed::folders() const
{
    return QVector<const Folder*>();
}

QVector<Folder*> Feed::folders()
{
    return QVector<Folder*>();
}

QVector<const Feed*> Feed::feeds() const
{
    QVector<const Feed*> list;
    list.append( this );
    return list;
}

QVector<Feed*> Feed::feeds()
{
    QVector<Feed*> list;
    list.append( this );
    return list;
}

Article Feed::findArticle(const QString& guid) const
{
    return d->articles[guid];
}

QList<Article> Feed::articles()
{
    if (!d->articlesLoaded)
        loadArticles();
    return d->articles.values();
}

Backend::Storage* Feed::storage()
{
    return d->storage;
}

void Feed::loadArticles()
{
    if (d->articlesLoaded)
        return;

    if (!d->archive)
        d->archive = d->storage->archiveFor(xmlUrl());

    QStringList list = d->archive->articles();
    for ( QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
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
    QList<Article> tarticles = articles();
    QList<Article>::ConstIterator it;
    QList<Article>::ConstIterator en = tarticles.constEnd();

    int oldUnread = d->archive->unread();

    int unread = 0;

    for (it = tarticles.constBegin(); it != en; ++it)
        if (!(*it).isDeleted() && (*it).status() != Read)
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

Feed::Private::Private( Backend::Storage* storage_, Feed* qq )
  : q( qq ),
    storage( storage_ ),
    autoFetch( false ),
    fetchInterval( 30 ),
    archiveMode( globalDefault ),
    maxArticleAge( 60 ),
    maxArticleNumber( 1000 ),
    markImmediatelyAsRead( false ),
    useNotification( false ),
    loadLinkedWebsite( false ),
    lastFetched( 0 ),
    fetchErrorCode( Syndication::Success ),
    fetchTries( 0 ),
    followDiscovery( false ),
    loader( 0 ),
    articlesLoaded( false ),
    archive( 0 ),
    totalCount( -1 )
{
    assert( q );
    assert( storage );
}

Feed::Feed( Backend::Storage* storage ) : TreeNode(), d( new Private( storage, this ) )
{
}

Feed::~Feed()
{
    FeedIconManager::self()->removeListener( this );
    slotAbortFetch();
    emitSignalDestroyed();
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

bool Feed::isFetching() const { return d->loader != 0; }

void Feed::setMarkImmediatelyAsRead(bool enabled)
{
    d->markImmediatelyAsRead = enabled;
    if (enabled)
        createMarkAsReadJob()->start();
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

QPixmap Feed::image() const { return d->imagePixmap; }

QString Feed::xmlUrl() const { return d->xmlUrl; }

void Feed::setXmlUrl(const QString& s)
{
    d->xmlUrl = s;
    if( ! Settings::fetchOnStartup() )
        QTimer::singleShot(KRandom::random() % 4000, this, SLOT(slotAddFeedIconListener())); // TODO: let's give a gui some time to show up before starting the fetch when no fetch on startup is used. replace this with something proper later...
}

QString Feed::htmlUrl() const { return d->htmlUrl; }

void Feed::setHtmlUrl(const QString& s) { d->htmlUrl = s; }

QString Feed::description() const { return d->description; }

void Feed::setDescription(const QString& s) { d->description = s; }

bool Feed::fetchErrorOccurred() const { return d->fetchErrorCode != Syndication::Success; }

Syndication::ErrorCode Feed::fetchErrorCode() const { return d->fetchErrorCode; }

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

KJob* Feed::createMarkAsReadJob()
{
    std::auto_ptr<ArticleModifyJob> job( new ArticleModifyJob );
    Q_FOREACH ( const Article& i, articles() )
    {
        const ArticleId aid = { xmlUrl(), i.guid() };
        job->setStatus( aid, Read );
    }
    return job.release();
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

void Feed::slotAddFeedIconListener()
{
    FeedIconManager::self()->addListener( KUrl( d->xmlUrl ), this );
}

void Feed::appendArticles(const Syndication::FeedPtr feed)
{
    d->setTotalCountDirty();
    bool changed = false;
    const bool notify = useNotification() || Settings::useNotifications();

    QList<ItemPtr> items = feed->items();
    QList<ItemPtr>::ConstIterator it = items.constBegin();
    QList<ItemPtr>::ConstIterator en = items.constEnd();


    int nudge=0;

    QList<Article> deletedArticles = d->deletedArticles;

    for ( ; it != en; ++it)
    {
        if ( !d->articles.contains((*it)->id()) ) // article not in list
        {
            Article mya(*it, this);
            mya.offsetPubDate(nudge);
            nudge--;
            appendArticle(mya);
            d->addedArticlesNotify.append(mya);

            if (!mya.isDeleted() && !markImmediatelyAsRead())
                mya.setStatus(New);
            else
                mya.setStatus(Read);
            if ( notify )
                NotificationManager::self()->slotNotifyArticle( mya );
            changed = true;
        }
        else // article is in list
        {
            // if the article's guid is no hash but an ID, we have to check if the article was updated. That's done by comparing the hash values.
            Article old = d->articles[(*it)->id()];
            Article mya(*it, this);
            if (!mya.guidIsHash() && mya.hash() != old.hash() && !old.isDeleted())
            {
                mya.setKeep(old.keep());
                int oldstatus = old.status();
                old.setStatus(Read);

                d->articles.remove(old.guid());
                appendArticle(mya);

                mya.setStatus(oldstatus);

                d->updatedArticlesNotify.append(mya);
                changed = true;
            }
            else if (old.isDeleted())
                deletedArticles.removeAll(mya);
        }
    }


    QList<Article>::ConstIterator dit = deletedArticles.constBegin();
    QList<Article>::ConstIterator dtmp;
    QList<Article>::ConstIterator den = deletedArticles.constEnd();

    // delete articles with delete flag set completely from archive, which aren't in the current feed source anymore
    while (dit != den)
    {
        dtmp = dit;
        ++dit;
        d->articles.remove((*dtmp).guid());
        d->archive->deleteArticle((*dtmp).guid());
        d->removedArticlesNotify.append( *dtmp );
        changed = true;
        d->deletedArticles.removeAll(*dtmp);
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
            if (!a.isDeleted() && a.status() != Read)
                setUnread(unread()+1);
        }
    }
}


void Feed::fetch(bool followDiscovery)
{
    d->followDiscovery = followDiscovery;
    d->fetchTries = 0;

    // mark all new as unread
    QList<Article> articles = d->articles.values();
    QList<Article>::Iterator it;
    QList<Article>::Iterator en = articles.end();
    for (it = articles.begin(); it != en; ++it)
    {
        if ((*it).status() == New)
        {
            (*it).setStatus(Unread);
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
    d->fetchErrorCode = Syndication::Success;

    d->loader = Syndication::Loader::create( this, SLOT(fetchCompleted(Syndication::Loader*,
                                                                       Syndication::FeedPtr,
                                                                       Syndication::ErrorCode)) );
    //connect(d->loader, SIGNAL(progress(unsigned long)), this, SLOT(slotSetProgress(unsigned long)));
    d->loader->loadFrom( d->xmlUrl);
}

void Feed::slotImageFetched(const QPixmap& image)
{
    setImage(image);
}

void Feed::fetchCompleted(Syndication::Loader *l, Syndication::FeedPtr doc, Syndication::ErrorCode status)
{
    // Note that loader instances delete themselves
    d->loader = 0;

    // fetching wasn't successful:
    if (status != Syndication::Success)
    {
        if (status == Syndication::Aborted)
        {
            d->fetchErrorCode = Syndication::Success;
            emit fetchAborted(this);
        }
        else if (d->followDiscovery && (status == Syndication::InvalidXml) && (d->fetchTries < 3) && (l->discoveredFeedURL().isValid()))
        {
            d->fetchTries++;
            d->xmlUrl = l->discoveredFeedURL().url();
            emit fetchDiscovery(this);
            tryFetch();
        }
        else
        {
            d->fetchErrorCode = status;
            emit fetchError(this);
        }
        return;
    }

    loadArticles(); // TODO: make me fly: make this delayed

    FeedIconManager::self()->addListener( KUrl( xmlUrl() ), this );

    d->fetchErrorCode = Syndication::Success;

    if (d->imagePixmap.isNull())
    {
        QString u = d->xmlUrl;
        QString imageFileName = KGlobal::dirs()->saveLocation("cache", "akregator/Media/")
                                + Utils::fileNameForUrl(d->xmlUrl) + ".png";
        d->imagePixmap=QPixmap(imageFileName, "PNG");

        // if we aint got teh image and the feed provides one, get it....
        // TODO: reenable image fetching!
        if (false) // d->imagePixmap.isNull() && doc.image())
        {
            //d->image = *doc.image();
            //connect(&d->image, SIGNAL(gotPixmap(const QPixmap&)), this, SLOT(slotImageFetched(const QPixmap&)));
            //d->image.getPixmap();
        }
    }

    if (title().isEmpty())
        setTitle( doc->title() );

    d->description = doc->description();
    d->htmlUrl = doc->link();

    appendArticles(doc);

    d->archive->setLastFetch( QDateTime::currentDateTime().toTime_t());
    emit fetched(this);
}

QIcon Feed::icon() const
{
    if ( fetchErrorOccurred() )
        return KIcon("dialog-error");

    return !d->favicon.isNull() ? d->favicon : KIcon("text-html");
}

void Feed::deleteExpiredArticles( ArticleDeleteJob* deleteJob )
{
    if ( !usesExpiryByAge() )
        return;

    setNotificationMode(false);

    const QList<Article> articles = d->articles.values();
    QList<ArticleId> toDelete;
    const QString feedUrl = xmlUrl();
    const bool useKeep = Settings::doNotExpireImportantArticles();

    Q_FOREACH ( const Article& i, articles )
    {
        if ( ( !useKeep || !i.keep() ) && isExpired( i ) )
        {
            const ArticleId aid = { feedUrl, i.guid() };
            toDelete.append( aid );
        }
    }

    deleteJob->appendArticleIds( toDelete );
    setNotificationMode(true);
}

void Feed::setFavicon( const QIcon& icon )
{
    d->favicon = icon;
    nodeModified();
}

void Feed::setImage(const QPixmap &p)
{
    if (p.isNull())
        return;
    d->imagePixmap=p;
    d->imagePixmap.save(KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+ Utils::fileNameForUrl(d->xmlUrl) + ".png","PNG");
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
    d->setTotalCountDirty();
    if (!d->deletedArticles.contains(a))
        d->deletedArticles.append(a);

    d->updatedArticlesNotify.append(a);
    articlesModified();
}

void Feed::setArticleChanged(Article& a, int oldStatus)
{
    if (oldStatus != -1)
    {
        int newStatus = a.status();
        if (oldStatus == Read && newStatus != Read)
            setUnread(unread()+1);
        else if (oldStatus != Read && newStatus == Read)
            setUnread(unread()-1);
    }
    d->updatedArticlesNotify.append(a);
    articlesModified();
}

int Feed::totalCount() const
{
    if ( d->totalCount == -1 )
        d->totalCount = std::count_if( d->articles.constBegin(), d->articles.constEnd(), !bind( &Article::isDeleted, _1 ) );
    return d->totalCount;
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


const TreeNode* Feed::next() const
{
    if ( nextSibling() )
        return nextSibling();

    const Folder* p = parent();
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
        // copy list, otherwise the refcounting in Article::Private breaks for
        // some reason (causing segfaults)
        QList<Article> l = d->addedArticlesNotify;
        emit signalArticlesAdded(this, l);
        d->addedArticlesNotify.clear();
    }
    if (!d->updatedArticlesNotify.isEmpty())
    {
        // copy list, otherwise the refcounting in Article::Private breaks for
        // some reason (causing segfaults)
        QList<Article> l = d->updatedArticlesNotify;
        emit signalArticlesUpdated(this, l);
        d->updatedArticlesNotify.clear();
    }
    if (!d->removedArticlesNotify.isEmpty())
    {
        // copy list, otherwise the refcounting in Article::Private breaks for
        // some reason (causing segfaults)
        QList<Article> l = d->removedArticlesNotify;
        emit signalArticlesRemoved(this, l);
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

    QList<Article> articles = d->articles.values();
    qSort(articles);

    int c = 0;
    const bool useKeep = Settings::doNotExpireImportantArticles();

    ArticleDeleteJob* job = new ArticleDeleteJob;
    Q_FOREACH ( const Article& i, articles )
    {
        if (c < limit)
        {
            if ( !i.isDeleted() && ( !useKeep || !i.keep() ) )
                ++c;
        }
        else if ( !useKeep || !i.keep() )
        {
            const ArticleId aid = { i.feed()->xmlUrl(), i.guid() };
            job->appendArticleId( aid );
        }
    }
    job->start();
}

#include "feed.moc"

