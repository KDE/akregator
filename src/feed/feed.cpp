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

#include "feed.h"

#include "akregatorconfig.h"
#include "article.h"
#include "articlejobs.h"
#include "feedstorage.h"
#include "fetchqueue.h"
#include "folder.h"
#include "notificationmanager.h"
#include "storage.h"
#include "treenodevisitor.h"
#include "types.h"
#include "utils.h"
#include "feedretriever.h"

#include <Syndication/Syndication>

#include "akregator_debug.h"
#include <QIcon>
#include <QFileInfo>
#include <QDir>

#include <QUrl>
#include <KRandom>

#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QList>
#include <QPixmap>
#include <QTimer>

#include <memory>
#include <QStandardPaths>
#include <KIO/FavIconRequestJob>

using Syndication::ItemPtr;
using namespace Akregator;

template<typename Key, typename Value, template<typename, typename> class Container>
QVector<Value> valuesToVector(const Container<Key, Value> &container)
{
    QVector<Value> values;
    values.reserve(container.size());
    for (const Value &value : container) {
        values << value;
    }
    return values;
}

class Q_DECL_HIDDEN Akregator::Feed::Private
{
    Akregator::Feed *const q;
public:
    explicit Private(Backend::Storage *storage, Akregator::Feed *qq);

    Backend::Storage *storage = nullptr;
    bool autoFetch = false;
    int fetchInterval;
    ArchiveMode archiveMode;
    int maxArticleAge;
    int maxArticleNumber;
    bool markImmediatelyAsRead = false;
    bool useNotification = false;
    bool loadLinkedWebsite = false;
    int lastFetched;

    Syndication::ErrorCode fetchErrorCode;
    int fetchTries;
    bool followDiscovery = false;
    Syndication::Loader *loader = nullptr;
    bool articlesLoaded = false;
    Backend::FeedStorage *archive = nullptr;

    QString xmlUrl;
    QString htmlUrl;
    QString description;
    QString comment;

    /** list of feed articles */
    QHash<QString, Article> articles;

    /** list of deleted articles. This contains **/
    QVector<Article> deletedArticles;

    /** caches guids of deleted articles for notification */

    QVector<Article> addedArticlesNotify;
    QVector<Article> removedArticlesNotify;
    QVector<Article> updatedArticlesNotify;

    QPixmap imagePixmap;
    Syndication::ImagePtr image;
    QIcon favicon;
    mutable int totalCount;
    void setTotalCountDirty() const
    {
        totalCount = -1;
    }
};

QString Feed::archiveModeToString(ArchiveMode mode)
{
    switch (mode) {
    case keepAllArticles:
        return QStringLiteral("keepAllArticles");
    case disableArchiving:
        return QStringLiteral("disableArchiving");
    case limitArticleNumber:
        return QStringLiteral("limitArticleNumber");
    case limitArticleAge:
        return QStringLiteral("limitArticleAge");
    default:
        break;
    }
    return QStringLiteral("globalDefault");
}

Akregator::Feed *Feed::fromOPML(const QDomElement &e, Backend::Storage *storage)
{
    if (!e.hasAttribute(QStringLiteral("xmlUrl")) && !e.hasAttribute(QStringLiteral("xmlurl")) && !e.hasAttribute(QStringLiteral("xmlURL"))) {
        return nullptr;
    }

    const QString title = e.hasAttribute(QStringLiteral("text")) ? e.attribute(QStringLiteral("text")) : e.attribute(QStringLiteral("title"));

    QString xmlUrl = e.hasAttribute(QStringLiteral("xmlUrl")) ? e.attribute(QStringLiteral("xmlUrl")) : e.attribute(QStringLiteral("xmlurl"));
    if (xmlUrl.isEmpty()) {
        xmlUrl = e.attribute(QStringLiteral("xmlURL"));
    }

    bool useCustomFetchInterval = e.attribute(QStringLiteral("useCustomFetchInterval")) == QLatin1String("true");

    const QString htmlUrl = e.attribute(QStringLiteral("htmlUrl"));
    const QString description = e.attribute(QStringLiteral("description"));
    const int fetchInterval = e.attribute(QStringLiteral("fetchInterval")).toInt();
    const ArchiveMode archiveMode = stringToArchiveMode(e.attribute(QStringLiteral("archiveMode")));
    const int maxArticleAge = e.attribute(QStringLiteral("maxArticleAge")).toUInt();
    const int maxArticleNumber = e.attribute(QStringLiteral("maxArticleNumber")).toUInt();
    const bool markImmediatelyAsRead = e.attribute(QStringLiteral("markImmediatelyAsRead")) == QLatin1String("true");
    const bool useNotification = e.attribute(QStringLiteral("useNotification")) == QLatin1String("true");
    const bool loadLinkedWebsite = e.attribute(QStringLiteral("loadLinkedWebsite")) == QLatin1String("true");
    const QString comment = e.attribute(QStringLiteral("comment"));
    const uint id = e.attribute(QStringLiteral("id")).toUInt();

    Feed *const feed = new Feed(storage);
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
    feed->setComment(comment);
    if (!feed->d->archive && storage) {
        // Instead of loading the articles, we use the cache from storage
        feed->d->archive = storage->archiveFor(xmlUrl);
        feed->d->totalCount = feed->d->archive->totalCount();
    }
    return feed;
}

bool Feed::accept(TreeNodeVisitor *visitor)
{
    if (visitor->visitFeed(this)) {
        return true;
    } else {
        return visitor->visitTreeNode(this);
    }
}

QVector<const Folder *> Feed::folders() const
{
    return QVector<const Folder *>();
}

QVector<Folder *> Feed::folders()
{
    return QVector<Folder *>();
}

QVector<const Akregator::Feed *> Feed::feeds() const
{
    QVector<const Akregator::Feed *> list;
    list.append(this);
    return list;
}

QVector<Akregator::Feed *> Feed::feeds()
{
    QVector<Feed *> list;
    list.append(this);
    return list;
}

Article Feed::findArticle(const QString &guid) const
{
    return d->articles.value(guid);
}

QVector<Article> Feed::articles()
{
    if (!d->articlesLoaded) {
        loadArticles();
    }
    return valuesToVector(d->articles);
}

Backend::Storage *Feed::storage()
{
    return d->storage;
}

void Feed::loadArticles()
{
    if (d->articlesLoaded) {
        return;
    }

    if (!d->archive && d->storage) {
        d->archive = d->storage->archiveFor(xmlUrl());
    }

    QStringList list = d->archive->articles();
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        Article mya(*it, this, d->archive);
        d->articles[mya.guid()] = mya;
        if (mya.isDeleted()) {
            d->deletedArticles.append(mya);
        }
    }

    d->articlesLoaded = true;
    enforceLimitArticleNumber();
    recalcUnreadCount();
}

void Feed::recalcUnreadCount()
{
    QVector<Article> tarticles = articles();
    QVector<Article>::ConstIterator it;
    QVector<Article>::ConstIterator en = tarticles.constEnd();

    int oldUnread = d->archive->unread();

    int unread = 0;

    for (it = tarticles.constBegin(); it != en; ++it) {
        if (!(*it).isDeleted() && (*it).status() != Read) {
            ++unread;
        }
    }

    if (unread != oldUnread) {
        d->archive->setUnread(unread);
        nodeModified();
    }
}

Feed::ArchiveMode Feed::stringToArchiveMode(const QString &str)
{
    if (str == QLatin1String("globalDefault")) {
        return globalDefault;
    } else if (str == QLatin1String("keepAllArticles")) {
        return keepAllArticles;
    } else if (str == QLatin1String("disableArchiving")) {
        return disableArchiving;
    } else if (str == QLatin1String("limitArticleNumber")) {
        return limitArticleNumber;
    } else if (str == QLatin1String("limitArticleAge")) {
        return limitArticleAge;
    }

    return globalDefault;
}

Feed::Private::Private(Backend::Storage *storage_, Akregator::Feed *qq)
    : q(qq)
    , storage(storage_)
    , autoFetch(false)
    , fetchInterval(30)
    , archiveMode(globalDefault)
    , maxArticleAge(60)
    , maxArticleNumber(1000)
    , markImmediatelyAsRead(false)
    , useNotification(false)
    , loadLinkedWebsite(false)
    , lastFetched(0)
    , fetchErrorCode(Syndication::Success)
    , fetchTries(0)
    , followDiscovery(false)
    , loader(nullptr)
    , articlesLoaded(false)
    , archive(nullptr)
    , totalCount(-1)
{
    Q_ASSERT(q);
    Q_ASSERT(storage);
}

Feed::Feed(Backend::Storage *storage) : TreeNode()
    , d(new Private(storage, this))
{
}

Feed::~Feed()
{
    slotAbortFetch();
    emitSignalDestroyed();
    delete d;
    d = nullptr;
}

void Feed::loadFavicon(const QUrl &url)
{
    KIO::FavIconRequestJob *job = new KIO::FavIconRequestJob(url);
    connect(job, &KIO::FavIconRequestJob::result, this, [job, this](KJob *) {
        if (!job->error()) {
            setFavicon(QIcon(job->iconFile()));
        }
    });
}

bool Feed::useCustomFetchInterval() const
{
    return d->autoFetch;
}

void Feed::setCustomFetchIntervalEnabled(bool enabled)
{
    d->autoFetch = enabled;
}

int Feed::fetchInterval() const
{
    return d->fetchInterval;
}

void Feed::setFetchInterval(int interval)
{
    d->fetchInterval = interval;
}

int Feed::maxArticleAge() const
{
    return d->maxArticleAge;
}

void Feed::setMaxArticleAge(int maxArticleAge)
{
    d->maxArticleAge = maxArticleAge;
}

int Feed::maxArticleNumber() const
{
    return d->maxArticleNumber;
}

void Feed::setMaxArticleNumber(int maxArticleNumber)
{
    d->maxArticleNumber = maxArticleNumber;
}

bool Feed::markImmediatelyAsRead() const
{
    return d->markImmediatelyAsRead;
}

bool Feed::isFetching() const
{
    return d->loader != nullptr;
}

void Feed::setMarkImmediatelyAsRead(bool enabled)
{
    d->markImmediatelyAsRead = enabled;
}

void Feed::setComment(const QString &comment)
{
    d->comment = comment;
}

QString Feed::comment() const
{
    return d->comment;
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

QPixmap Feed::image() const
{
    return d->imagePixmap;
}

QString Feed::xmlUrl() const
{
    return d->xmlUrl;
}

void Feed::setXmlUrl(const QString &s)
{
    d->xmlUrl = s;
    if (!Settings::fetchOnStartup()) {
        QTimer::singleShot(KRandom::random() % 4000, this, &Feed::slotAddFeedIconListener);    // TODO: let's give a gui some time to show up before starting the fetch when no fetch on startup is used. replace this with something proper later...
    }
}

QString Feed::htmlUrl() const
{
    return d->htmlUrl;
}

void Feed::setHtmlUrl(const QString &s)
{
    d->htmlUrl = s;
}

QString Feed::description() const
{
    return d->description;
}

void Feed::setDescription(const QString &s)
{
    d->description = s;
}

bool Feed::fetchErrorOccurred() const
{
    return d->fetchErrorCode != Syndication::Success;
}

Syndication::ErrorCode Feed::fetchErrorCode() const
{
    return d->fetchErrorCode;
}

bool Feed::isArticlesLoaded() const
{
    return d->articlesLoaded;
}

QDomElement Feed::toOPML(QDomElement parent, QDomDocument document) const
{
    QDomElement el = document.createElement(QStringLiteral("outline"));
    el.setAttribute(QStringLiteral("text"), title());
    el.setAttribute(QStringLiteral("title"), title());
    el.setAttribute(QStringLiteral("xmlUrl"), d->xmlUrl);
    el.setAttribute(QStringLiteral("htmlUrl"), d->htmlUrl);
    el.setAttribute(QStringLiteral("id"), QString::number(id()));
    el.setAttribute(QStringLiteral("description"), d->description);
    el.setAttribute(QStringLiteral("useCustomFetchInterval"), (useCustomFetchInterval() ? QStringLiteral("true") : QStringLiteral("false")));
    el.setAttribute(QStringLiteral("fetchInterval"), QString::number(fetchInterval()));
    el.setAttribute(QStringLiteral("archiveMode"), archiveModeToString(d->archiveMode));
    el.setAttribute(QStringLiteral("maxArticleAge"), d->maxArticleAge);
    el.setAttribute(QStringLiteral("comment"), d->comment);
    el.setAttribute(QStringLiteral("maxArticleNumber"), d->maxArticleNumber);
    if (d->markImmediatelyAsRead) {
        el.setAttribute(QStringLiteral("markImmediatelyAsRead"), QStringLiteral("true"));
    }
    if (d->useNotification) {
        el.setAttribute(QStringLiteral("useNotification"), QStringLiteral("true"));
    }
    if (d->loadLinkedWebsite) {
        el.setAttribute(QStringLiteral("loadLinkedWebsite"), QStringLiteral("true"));
    }
    el.setAttribute(QStringLiteral("maxArticleNumber"), d->maxArticleNumber);
    el.setAttribute(QStringLiteral("type"), QStringLiteral("rss"));   // despite some additional fields, it is still "rss" OPML
    el.setAttribute(QStringLiteral("version"), QStringLiteral("RSS"));
    parent.appendChild(el);
    return el;
}

KJob *Feed::createMarkAsReadJob()
{
    ArticleModifyJob *job = new ArticleModifyJob;
    const auto arts = articles();
    for (const Article &i : arts) {
        const ArticleId aid = { xmlUrl(), i.guid() };
        job->setStatus(aid, Read);
    }
    return job;
}

void Feed::slotAddToFetchQueue(FetchQueue *queue, bool intervalFetchOnly)
{
    if (!intervalFetchOnly) {
        queue->addFeed(this);
    } else {
        int interval = -1;

        if (useCustomFetchInterval()) {
            interval = fetchInterval() * 60;
        } else if (Settings::useIntervalFetch()) {
            interval = Settings::autoFetchInterval() * 60;
        }

        uint lastFetch = d->archive->lastFetch().toTime_t();

        uint now = QDateTime::currentDateTimeUtc().toTime_t();

        if (interval > 0 && (now - lastFetch) >= static_cast<uint>(interval)) {
            queue->addFeed(this);
        }
    }
}

void Feed::slotAddFeedIconListener()
{
    loadFavicon(QUrl(d->xmlUrl));
}

void Feed::appendArticles(const Syndication::FeedPtr &feed)
{
    d->setTotalCountDirty();
    bool changed = false;
    const bool notify = useNotification() || Settings::useNotifications();

    QList<ItemPtr> items = feed->items();
    QList<ItemPtr>::ConstIterator it = items.constBegin();
    QList<ItemPtr>::ConstIterator en = items.constEnd();

    int nudge = 0;

    QVector<Article> deletedArticles = d->deletedArticles;

    for (; it != en; ++it) {
        if (!d->articles.contains((*it)->id())) { // article not in list
            Article mya(*it, this);
            mya.offsetPubDate(nudge);
            nudge--;
            appendArticle(mya);
            d->addedArticlesNotify.append(mya);

            if (!mya.isDeleted() && !markImmediatelyAsRead()) {
                mya.setStatus(New);
            } else {
                mya.setStatus(Read);
            }
            if (notify) {
                NotificationManager::self()->slotNotifyArticle(mya);
            }
            changed = true;
        } else { // article is in list
            // if the article's guid is no hash but an ID, we have to check if the article was updated. That's done by comparing the hash values.
            Article old = d->articles[(*it)->id()];
            Article mya(*it, this);
            if (!mya.guidIsHash() && mya.hash() != old.hash() && !old.isDeleted()) {
                mya.setKeep(old.keep());
                int oldstatus = old.status();
                old.setStatus(Read);

                d->articles.remove(old.guid());
                appendArticle(mya);

                mya.setStatus(oldstatus);

                d->updatedArticlesNotify.append(mya);
                changed = true;
            } else if (old.isDeleted()) {
                deletedArticles.removeAll(mya);
            }
        }
    }

    QVector<Article>::ConstIterator dit = deletedArticles.constBegin();
    QVector<Article>::ConstIterator dtmp;
    QVector<Article>::ConstIterator den = deletedArticles.constEnd();

    // delete articles with delete flag set completely from archive, which aren't in the current feed source anymore
    while (dit != den) {
        dtmp = dit;
        ++dit;
        d->articles.remove((*dtmp).guid());
        d->archive->deleteArticle((*dtmp).guid());
        d->removedArticlesNotify.append(*dtmp);
        changed = true;
        d->deletedArticles.removeAll(*dtmp);
    }

    if (changed) {
        articlesModified();
    }
}

bool Feed::usesExpiryByAge() const
{
    return (d->archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleAge) || d->archiveMode == limitArticleAge;
}

bool Feed::isExpired(const Article &a) const
{
    QDateTime now = QDateTime::currentDateTime();
    int expiryAge = -1;
// check whether the feed uses the global default and the default is limitArticleAge
    if (d->archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleAge) {
        expiryAge = Settings::maxArticleAge() * 24 * 3600;
    } else // otherwise check if this feed has limitArticleAge set
    if (d->archiveMode == limitArticleAge) {
        expiryAge = d->maxArticleAge * 24 * 3600;
    }

    return expiryAge != -1 && a.pubDate().secsTo(now) > expiryAge;
}

void Feed::appendArticle(const Article &a)
{
    if ((a.keep() && Settings::doNotExpireImportantArticles()) || (!usesExpiryByAge() || !isExpired(a))) {   // if not expired
        if (!d->articles.contains(a.guid())) {
            d->articles[a.guid()] = a;
            if (!a.isDeleted() && a.status() != Read) {
                setUnread(unread() + 1);
            }
        }
    }
}

void Feed::fetch(bool followDiscovery)
{
    d->followDiscovery = followDiscovery;
    d->fetchTries = 0;

    // mark all new as unread
    for (auto it = d->articles.begin(), end = d->articles.end(); it != end; ++it) {
        if ((*it).status() == New) {
            (*it).setStatus(Unread);
        }
    }

    Q_EMIT fetchStarted(this);

    tryFetch();
}

void Feed::slotAbortFetch()
{
    if (d->loader) {
        d->loader->abort();
    }
}

void Feed::tryFetch()
{
    d->fetchErrorCode = Syndication::Success;

    d->loader = Syndication::Loader::create(this, SLOT(fetchCompleted(Syndication::Loader*,
                                                                      Syndication::FeedPtr,
                                                                      Syndication::ErrorCode)));
    d->loader->loadFrom(QUrl(d->xmlUrl), new FeedRetriever());
}

void Feed::slotImageFetched(const QPixmap &image)
{
    setImage(image);
}

void Feed::fetchCompleted(Syndication::Loader *l, Syndication::FeedPtr doc, Syndication::ErrorCode status)
{
    // Note that loader instances delete themselves
    d->loader = nullptr;

    // fetching wasn't successful:
    if (status != Syndication::Success) {
        if (status == Syndication::Aborted) {
            d->fetchErrorCode = Syndication::Success;
            Q_EMIT fetchAborted(this);
        } else if (d->followDiscovery && (status == Syndication::InvalidXml) && (d->fetchTries < 3) && (l->discoveredFeedURL().isValid())) {
            d->fetchTries++;
            d->xmlUrl = l->discoveredFeedURL().url();
            Q_EMIT fetchDiscovery(this);
            tryFetch();
        } else {
            d->fetchErrorCode = status;
            Q_EMIT fetchError(this);
        }
        markAsFetchedNow();
        return;
    }

    loadArticles(); // TODO: make me fly: make this delayed

    loadFavicon(QUrl(xmlUrl()));

    d->fetchErrorCode = Syndication::Success;

    if (d->imagePixmap.isNull()) {
        const QString imageFileName = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/akregator/Media/") + Utils::fileNameForUrl(d->xmlUrl) + QLatin1String(".png");
        d->imagePixmap = QPixmap(imageFileName, "PNG");
    }

    if (title().isEmpty()) {
        setTitle(Syndication::htmlToPlainText(doc->title()));
    }

    d->description = doc->description();
    d->htmlUrl = doc->link();

    appendArticles(doc);

    markAsFetchedNow();
    Q_EMIT fetched(this);
}

void Feed::markAsFetchedNow()
{
    if (d->archive) {
        d->archive->setLastFetch(QDateTime::currentDateTimeUtc());
    }
}

QIcon Feed::icon() const
{
    if (fetchErrorOccurred()) {
        return QIcon::fromTheme(QStringLiteral("dialog-error"));
    }

    return !d->favicon.isNull() ? d->favicon : QIcon::fromTheme(QStringLiteral("text-html"));
}

void Feed::deleteExpiredArticles(ArticleDeleteJob *deleteJob)
{
    if (!usesExpiryByAge()) {
        return;
    }

    setNotificationMode(false);

    QList<ArticleId> toDelete;
    const QString feedUrl = xmlUrl();
    const bool useKeep = Settings::doNotExpireImportantArticles();

    for (const Article &i : qAsConst(d->articles)) {
        if ((!useKeep || !i.keep()) && isExpired(i)) {
            const ArticleId aid = { feedUrl, i.guid() };
            toDelete.append(aid);
        }
    }

    deleteJob->appendArticleIds(toDelete);
    setNotificationMode(true);
}

void Feed::setFavicon(const QIcon &icon)
{
    d->favicon = icon;
    nodeModified();
}

void Feed::setImage(const QPixmap &p)
{
    if (p.isNull()) {
        return;
    }
    d->imagePixmap = p;
    const QString filename = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1String("/akregator/Media/") + Utils::fileNameForUrl(d->xmlUrl) + QLatin1String(".png");
    QFileInfo fileInfo(filename);
    QDir().mkpath(fileInfo.absolutePath());
    d->imagePixmap.save(filename, "PNG");
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
    if (d->archive && unread != d->archive->unread()) {
        d->archive->setUnread(unread);
        nodeModified();
    }
}

void Feed::setArticleDeleted(Article &a)
{
    d->setTotalCountDirty();
    if (!d->deletedArticles.contains(a)) {
        d->deletedArticles.append(a);
    }

    d->updatedArticlesNotify.append(a);
    articlesModified();
}

void Feed::setArticleChanged(Article &a, int oldStatus, bool process)
{
    int newStatus = a.status();
    if (oldStatus != -1) {
        if (oldStatus == Read && newStatus != Read) {
            setUnread(unread() + 1);
        } else if (oldStatus != Read && newStatus == Read) {
            setUnread(unread() - 1);
        }
    }
    d->updatedArticlesNotify.append(a);
    if (process) {
        articlesModified();
    }
}

int Feed::totalCount() const
{
    if (d->totalCount == -1) {
        d->totalCount = std::count_if(d->articles.constBegin(), d->articles.constEnd(), [](const Article &art) -> bool {
            return !art.isDeleted();
        });
    }
    return d->totalCount;
}

TreeNode *Feed::next()
{
    if (nextSibling()) {
        return nextSibling();
    }

    Folder *p = parent();
    while (p) {
        if (p->nextSibling()) {
            return p->nextSibling();
        } else {
            p = p->parent();
        }
    }
    return nullptr;
}

const TreeNode *Feed::next() const
{
    if (nextSibling()) {
        return nextSibling();
    }

    const Folder *p = parent();
    while (p) {
        if (p->nextSibling()) {
            return p->nextSibling();
        } else {
            p = p->parent();
        }
    }
    return nullptr;
}

void Feed::doArticleNotification()
{
    if (!d->addedArticlesNotify.isEmpty()) {
        // copy list, otherwise the refcounting in Article::Private breaks for
        // some reason (causing segfaults)
        QVector<Article> l = d->addedArticlesNotify;
        Q_EMIT signalArticlesAdded(this, l);
        d->addedArticlesNotify.clear();
    }
    if (!d->updatedArticlesNotify.isEmpty()) {
        // copy list, otherwise the refcounting in Article::Private breaks for
        // some reason (causing segfaults)
        QVector<Article> l = d->updatedArticlesNotify;
        Q_EMIT signalArticlesUpdated(this, l);
        d->updatedArticlesNotify.clear();
    }
    if (!d->removedArticlesNotify.isEmpty()) {
        // copy list, otherwise the refcounting in Article::Private breaks for
        // some reason (causing segfaults)
        QVector<Article> l = d->removedArticlesNotify;
        Q_EMIT signalArticlesRemoved(this, l);
        d->removedArticlesNotify.clear();
    }
    TreeNode::doArticleNotification();
}

void Feed::enforceLimitArticleNumber()
{
    int limit = -1;
    if (d->archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleNumber) {
        limit = Settings::maxArticleNumber();
    } else if (d->archiveMode == limitArticleNumber) {
        limit = maxArticleNumber();
    }

    if (limit == -1 || limit >= d->articles.count() - d->deletedArticles.count()) {
        return;
    }

    QVector<Article> articles = valuesToVector(d->articles);
    std::sort(articles.begin(), articles.end());

    int c = 0;
    const bool useKeep = Settings::doNotExpireImportantArticles();

    for (Article i : qAsConst(articles)) {
        if (c < limit) {
            if (!i.isDeleted() && (!useKeep || !i.keep())) {
                ++c;
            }
        } else if (!useKeep || !i.keep()) {
            i.setDeleted();
        }
    }
}
