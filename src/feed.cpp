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

#include <kurl.h>
#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <klocale.h>

#include <libkdepim/progressmanager.h>

#include "akregatorconfig.h"
#include "feed.h"
#include "feedgroup.h"
#include "fetchtransaction.h"

#include "feedstorage.h"
#include "storage.h"

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

        bool useCustomFetchInterval = e.attribute("y") == "true";

        QString htmlUrl = e.attribute("htmlUrl");
        QString description = e.attribute("description");
        int fetchInterval = e.attribute("fetchInterval").toUInt();
        ArchiveMode archiveMode = stringToArchiveMode(e.attribute("archiveMode"));
        int maxArticleAge = e.attribute("maxArticleAge").toUInt();
        int maxArticleNumber = e.attribute("maxArticleNumber").toUInt();
        bool markImmediatelyAsRead = e.attribute("markImmediatelyAsRead") == "true";
        bool useNotification = e.attribute("useNotification") == "true";
        uint id = e.attribute("id").toUInt();

        feed = new Feed();
        feed->setTitle(title);
        feed->setXmlUrl(xmlUrl);
        feed->setCustomFetchIntervalEnabled(useCustomFetchInterval);
        feed->setHtmlUrl(htmlUrl);
        feed->setId(id);
        feed->setDescription(description);
        feed->setArchiveMode(archiveMode);
        feed->setFetchInterval(fetchInterval);
        feed->setMaxArticleAge(maxArticleAge);
        feed->setMaxArticleNumber(maxArticleNumber);
        feed->setMarkImmediatelyAsRead(markImmediatelyAsRead);
    }

    feed->loadArticles(); // TODO: make me fly: make this delayed
    return feed;
}

ArticleSequence Feed::articles()
{
    if (!m_articlesLoaded)
        loadArticles();
    return m_articles;
}

void Feed::loadArticles()
{
    if (m_articlesLoaded)
        return;

    if (!m_archive)
        m_archive = Backend::Storage::getInstance()->archiveFor(xmlUrl());

    m_articles.enableSorting(false);
    QStringList list = m_archive->articles();
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it)
    {
        MyArticle mya(*it, this);
        m_articles.append(mya);
        if (mya.isDeleted())
            m_deletedArticles.append(mya);
    }
    m_articles.enableSorting(true);
    m_articles.sort();
    m_articlesLoaded = true;
    enforceLimitArticleNumber();
    recalcUnreadCount();
}

void Feed::recalcUnreadCount()
{
    ArticleSequence tarticles = articles();
    ArticleSequence::Iterator it;
    ArticleSequence::Iterator en = tarticles.end();

    int unread = 0;
    for (it = tarticles.begin(); it != en; ++it)
        if (!(*it).isDeleted() && (*it).status() != MyArticle::Read)
            ++unread;
    m_archive->setUnread(unread);
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
    , m_autoFetch(false)
    , m_fetchInterval(30)
    , m_archiveMode(globalDefault)
    , m_maxArticleAge(60)
    , m_maxArticleNumber(1000)
    , m_markImmediatelyAsRead(false)
    , m_useNotification(false)
    , m_transaction(0)
    , m_fetchError(false)
    , m_fetchTries(0)
    , m_loader(0)
    , m_articlesLoaded(false)
    , m_archive(0)
    , m_articles()
    , m_progressItem(0)
{
}

Feed::~Feed()
{
    slotAbortFetch();
    // tell the world that this node is destroyed
    emit signalDestroyed(this);
}

QDomElement Feed::toOPML( QDomElement parent, QDomDocument document ) const
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    el.setAttribute( "title", title() );
    el.setAttribute( "xmlUrl", m_xmlUrl );
    el.setAttribute( "htmlUrl", m_htmlUrl );
    el.setAttribute( "id", QString::number(id()) );
    el.setAttribute( "description", m_description );
    el.setAttribute( "autoFetch", (useCustomFetchInterval() ? "true" : "false") );
    el.setAttribute( "fetchInterval", QString::number(fetchInterval()) );
    el.setAttribute( "archiveMode", archiveModeToString(m_archiveMode) );
    el.setAttribute( "maxArticleAge", m_maxArticleAge );
    el.setAttribute( "maxArticleNumber", m_maxArticleNumber );
    if (m_markImmediatelyAsRead)
        el.setAttribute( "markImmediatelyAsRead", "true" );
    if (m_useNotification)
        el.setAttribute( "useNotification", "true" );
    el.setAttribute( "maxArticleNumber", m_maxArticleNumber );
    el.setAttribute( "type", "rss" ); // despite some additional fields, its still "rss" OPML
    el.setAttribute( "version", "RSS" );
    parent.appendChild( el );
    return el;
}

void Feed::slotMarkAllArticlesAsRead()
{
    if (unread() > 0)
    {
        ArticleSequence tarticles = articles();
        ArticleSequence::Iterator it;
        ArticleSequence::Iterator en = tarticles.end();

        for (it = tarticles.begin(); it != en; ++it)
            (*it).setStatus(MyArticle::Read);

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

    ArticleSequence deletedArticles = m_deletedArticles;
    
    for (it = d_articles.begin(); it != en; ++it)
    {
        MyArticle mya(*it, this);

        ArticleSequence::Iterator old = m_articles.find(mya);

        if ( old == m_articles.end() ) // article not in list
        {
            if (!markImmediatelyAsRead())
                mya.setStatus(MyArticle::New);
            else
                mya.setStatus(MyArticle::Read);

            mya.offsetFetchTime(nudge);
            nudge--;
            appendArticle(mya);
            changed = true;
        }
        // if the article's guid is no hash but an ID, we have to check if the article was updated. That's done by comparing the hash values.
        else if (!mya.guidIsHash() && mya.hash() != (*old).hash() && !mya.isDeleted())
        {
            mya.setKeep((*old).keep());
            // reset status to New
            mya.setStatus(MyArticle::New);
            m_articles.remove(old);
            appendArticle(mya);
            changed = true;
        }
        else if (mya.isDeleted())
            deletedArticles.remove(mya);
    }

    ArticleSequence::ConstIterator dit = deletedArticles.begin();
    ArticleSequence::ConstIterator dtmp;
    ArticleSequence::ConstIterator den = deletedArticles.end();

    // delete articles with delete flag set completely from archive, which aren't in the current feed source anymore
    while (dit != den)
    {
        dtmp = dit;
        ++dit;
        m_articles.remove(*dtmp);
        m_archive->deleteArticle((*dtmp).guid());
        m_deletedArticles.remove(*dtmp);
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
        {
            setUnread(unread()+1);
        }
        if (!m_articles.contains(a))
            m_articles.append(a);
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

void Feed::slotSetProgress(unsigned long percent)
{
    if (m_progressItem)
        m_progressItem->setProgress((unsigned int) percent);
}

void Feed::slotAbortFetch()
{
    if (m_loader)
        m_loader->abort();
}

void Feed::tryFetch()
{
    m_fetchError = false;

    m_progressItem = KPIM::ProgressManager::createProgressItem(KPIM::ProgressManager::getUniqueID(), title(), QString::null, false);
    //connect(m_progressItem, SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), SLOT(slotAbortFetch()));

    m_loader = Loader::create( this, SLOT(fetchCompleted(Loader *, Document, Status)) );
    //connect(m_loader, SIGNAL(progress(unsigned long)), this, SLOT(slotSetProgress(unsigned long)));
    m_loader->loadFrom( m_xmlUrl, new FileRetriever );
}

void Feed::fetchCompleted(Loader *l, Document doc, Status status)
{
    // Note that loader instances delete themselves
    m_loader = 0;

    // fetching wasn't successful:
    if (status != Success)
    {
        if(m_progressItem)
        {
            switch (status)
            {
                case RetrieveError:
                    m_progressItem->setStatus(i18n("Feed file is not available"));
                    break;
                case ParseError:
                    m_progressItem->setStatus(i18n("Parsing of feed file failed"));
                    break;
                case Aborted:
                    m_progressItem->setStatus(i18n("Fetch aborted"));
                    break;
                default:
                    break;
            }

            m_progressItem->setComplete();
            m_progressItem = 0;
        }

        m_transaction = 0;

        if (status == Aborted)
        {
            m_fetchError = false;
            emit fetchAborted(this);
        }
        else if (m_followDiscovery && (status == ParseError) && (m_fetchTries < 3) && (l->discoveredFeedURL().isValid()))
        {
            m_fetchTries++;
            m_xmlUrl = l->discoveredFeedURL().url();
            emit fetchDiscovery(this);
            tryFetch();
        }
        else
        {
            m_fetchError = true;
            emit fetchError(this);
        }
        return;
    }

    loadArticles(); // TODO: make me fly: make this delayed

    if (m_progressItem)
    {
        m_progressItem->setComplete();
        m_progressItem = 0;
    }

    // Restore favicon.
    if (m_favicon.isNull())
        loadFavicon();

    m_fetchError = false;
    
    if (m_image.isNull())
    {
        QString u = m_xmlUrl;
        QString imageFileName = KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png";
        m_image=QPixmap(imageFileName, "PNG");

        // if we aint got teh image and the feed provides one, get it....
        if (m_image.isNull() && doc.image() && m_transaction)
            m_transaction->addImage(this, doc.image());
    }

    if (title().isEmpty())
        setTitle( KCharsets::resolveEntities(KCharsets::resolveEntities(doc.title())) );

    m_description = doc.description();
    m_htmlUrl = doc.link().url();

    appendArticles(doc);

    m_transaction = 0;
    m_archive->setLastFetch( QDateTime::currentDateTime().toTime_t());
    emit fetched(this);
}

void Feed::loadFavicon()
{
    if (!m_transaction)
       return;
    m_transaction->addIcon(this);
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
                m_archive->deleteArticle((*tmp).guid());
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

int Feed::unread() const
{
    return m_archive ? m_archive->unread() : 0;
}

void Feed::setUnread(int unread)
{
    if (m_archive && unread != m_archive->unread())
    {
        m_archive->setUnread(unread);
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

void Feed::enforceLimitArticleNumber()
{
    int limit = -1;
    if (m_archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleNumber)
        limit = Settings::maxArticleNumber();
    else if (m_archiveMode == limitArticleNumber)
        limit = maxArticleNumber();
        
    if (limit == -1 || limit >= m_articles.count() - m_deletedArticles.count())
        return;
        
    bool changed = false;

    ArticleSequence::Iterator it = m_articles.begin();
    ArticleSequence::Iterator tmp;
    ArticleSequence::Iterator en = m_articles.end();

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
        else
        {
            if (!(*tmp).keep())
            {
                (*tmp).setDeleted();
                changed = true;
            }
        }
    }
    
    if (changed)
        modified();
}

void Feed::setArticleDeleted(const MyArticle& mya)
{
    if (!m_deletedArticles.contains(mya))
        m_deletedArticles.append(mya);
}

#include "feed.moc"
