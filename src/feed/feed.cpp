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
#include "job/downloadfeediconjob.h"
#include <syndication_version.h>
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

    Backend::Storage *m_storage = nullptr;
    bool m_autoFetch = false;
    int m_fetchInterval;
    ArchiveMode m_archiveMode;
    int m_maxArticleAge;
    int m_maxArticleNumber;
    bool m_markImmediatelyAsRead = false;
    bool m_useNotification = false;
    bool m_loadLinkedWebsite = false;

    Syndication::ErrorCode m_fetchErrorCode;
    int m_fetchTries;
    bool m_followDiscovery = false;
    Syndication::Loader *m_loader = nullptr;
    bool m_articlesLoaded = false;
    Backend::FeedStorage *m_archive = nullptr;

    QString m_xmlUrl;
    QString m_htmlUrl;
    QString m_description;
    QString m_comment;

    /** list of feed articles */
    QHash<QString, Article> articles;

    /** list of deleted articles. This contains **/
    QVector<Article> m_deletedArticles;

    /** caches guids of deleted articles for notification */

    QVector<Article> m_addedArticlesNotify;
    QVector<Article> m_removedArticlesNotify;
    QVector<Article> m_updatedArticlesNotify;

    Feed::ImageInfo m_logoInfo;
    Feed::ImageInfo m_faviconInfo;

    QIcon m_favicon;
    mutable int m_totalCount;
    void setTotalCountDirty() const;
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
    const QString faviconUrl = e.attribute(QStringLiteral("faviconUrl"));
    Feed::ImageInfo faviconInfo;
    faviconInfo.imageUrl = faviconUrl;
    if (e.hasAttribute(QStringLiteral("faviconWidth"))) {
        faviconInfo.width = e.attribute(QStringLiteral("faviconWidth")).toInt();
    }
    if (e.hasAttribute(QStringLiteral("faviconHeight"))) {
        faviconInfo.height = e.attribute(QStringLiteral("faviconHeight")).toInt();
    }

    Feed::ImageInfo logoInfo;
    const QString logoUrl = e.attribute(QStringLiteral("logoUrl"));
    logoInfo.imageUrl = logoUrl;
    if (e.hasAttribute(QStringLiteral("logoWidth"))) {
        logoInfo.width = e.attribute(QStringLiteral("logoWidth")).toInt();
    }
    if (e.hasAttribute(QStringLiteral("logoHeight"))) {
        logoInfo.height = e.attribute(QStringLiteral("logoHeight")).toInt();
    }

    const uint id = e.attribute(QStringLiteral("id")).toUInt();

    Feed *const feed = new Feed(storage);
    feed->setTitle(title);
    feed->setFaviconInfo(faviconInfo);
    feed->setLogoInfo(logoInfo);

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
    if (!feed->d->m_archive && storage) {
        // Instead of loading the articles, we use the cache from storage
        feed->d->m_archive = storage->archiveFor(xmlUrl);
        feed->d->m_totalCount = feed->d->m_archive->totalCount();
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
    if (!d->m_articlesLoaded) {
        loadArticles();
    }
    return valuesToVector(d->articles);
}

Backend::Storage *Feed::storage()
{
    return d->m_storage;
}

void Feed::loadArticles()
{
    if (d->m_articlesLoaded) {
        return;
    }

    if (!d->m_archive && d->m_storage) {
        d->m_archive = d->m_storage->archiveFor(xmlUrl());
    }

    const QStringList list = d->m_archive->articles();
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        Article mya(*it, this, d->m_archive);
        d->articles[mya.guid()] = mya;
        if (mya.isDeleted()) {
            d->m_deletedArticles.append(mya);
        }
    }

    d->m_articlesLoaded = true;
    enforceLimitArticleNumber();
    recalcUnreadCount();
}

void Feed::recalcUnreadCount()
{
    QVector<Article> tarticles = articles();
    QVector<Article>::ConstIterator it;
    QVector<Article>::ConstIterator en = tarticles.constEnd();

    int oldUnread = d->m_archive->unread();

    int unread = 0;

    for (it = tarticles.constBegin(); it != en; ++it) {
        if (!(*it).isDeleted() && (*it).status() != Read) {
            ++unread;
        }
    }

    if (unread != oldUnread) {
        d->m_archive->setUnread(unread);
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
    , m_storage(storage_)
    , m_autoFetch(false)
    , m_fetchInterval(30)
    , m_archiveMode(globalDefault)
    , m_maxArticleAge(60)
    , m_maxArticleNumber(1000)
    , m_markImmediatelyAsRead(false)
    , m_useNotification(false)
    , m_loadLinkedWebsite(false)
    , m_fetchErrorCode(Syndication::Success)
    , m_fetchTries(0)
    , m_followDiscovery(false)
    , m_loader(nullptr)
    , m_articlesLoaded(false)
    , m_archive(nullptr)
    , m_totalCount(-1)
{
    Q_ASSERT(q);
    Q_ASSERT(m_storage);
}

void Feed::Private::setTotalCountDirty() const
{
    m_totalCount = -1;
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

void Feed::loadFavicon(const QString &url, bool downloadFavicon)
{
    Akregator::DownloadFeedIconJob *job = new Akregator::DownloadFeedIconJob(this);
    job->setFeedIconUrl(url);
    job->setDownloadFavicon(downloadFavicon);
    connect(job, &DownloadFeedIconJob::result, this, [job, this](const QString &result) {
        setFaviconUrl(result);
    });
    if (!job->start()) {
        qCWarning(AKREGATOR_LOG) << "Impossible to start DownloadFeedIconJob for url: " << url;
    }
}

bool Feed::useCustomFetchInterval() const
{
    return d->m_autoFetch;
}

void Feed::setCustomFetchIntervalEnabled(bool enabled)
{
    d->m_autoFetch = enabled;
}

int Feed::fetchInterval() const
{
    return d->m_fetchInterval;
}

void Feed::setFetchInterval(int interval)
{
    d->m_fetchInterval = interval;
}

int Feed::maxArticleAge() const
{
    return d->m_maxArticleAge;
}

void Feed::setMaxArticleAge(int maxArticleAge)
{
    d->m_maxArticleAge = maxArticleAge;
}

int Feed::maxArticleNumber() const
{
    return d->m_maxArticleNumber;
}

void Feed::setMaxArticleNumber(int maxArticleNumber)
{
    d->m_maxArticleNumber = maxArticleNumber;
}

bool Feed::markImmediatelyAsRead() const
{
    return d->m_markImmediatelyAsRead;
}

bool Feed::isFetching() const
{
    return d->m_loader != nullptr;
}

void Feed::setMarkImmediatelyAsRead(bool enabled)
{
    d->m_markImmediatelyAsRead = enabled;
}

void Feed::setComment(const QString &comment)
{
    d->m_comment = comment;
}

QString Feed::comment() const
{
    return d->m_comment;
}

void Feed::setUseNotification(bool enabled)
{
    d->m_useNotification = enabled;
}

bool Feed::useNotification() const
{
    return d->m_useNotification;
}

void Feed::setLoadLinkedWebsite(bool enabled)
{
    d->m_loadLinkedWebsite = enabled;
}

bool Feed::loadLinkedWebsite() const
{
    return d->m_loadLinkedWebsite;
}

Feed::ImageInfo Feed::logoInfo() const
{
    return d->m_logoInfo;
}

QString Feed::xmlUrl() const
{
    return d->m_xmlUrl;
}

void Feed::setXmlUrl(const QString &s)
{
    d->m_xmlUrl = s;
    if (!Settings::fetchOnStartup()) {
        QTimer::singleShot(KRandom::random() % 4000, this, &Feed::slotAddFeedIconListener);    // TODO: let's give a gui some time to show up before starting the fetch when no fetch on startup is used. replace this with something proper later...
    }
}

QString Feed::htmlUrl() const
{
    return d->m_htmlUrl;
}

void Feed::setHtmlUrl(const QString &s)
{
    d->m_htmlUrl = s;
}

Feed::ImageInfo Feed::faviconInfo() const
{
    return d->m_faviconInfo;
}

void Feed::setFaviconUrl(const QString &url)
{
    d->m_faviconInfo.imageUrl = url;
    setFavicon(QIcon(d->m_faviconInfo.imageUrl));
}

void Feed::setFaviconInfo(const Feed::ImageInfo &info)
{
    d->m_faviconInfo = info;
    setFavicon(QIcon(info.imageUrl));
}

QString Feed::description() const
{
    return d->m_description;
}

void Feed::setDescription(const QString &s)
{
    d->m_description = s;
}

bool Feed::fetchErrorOccurred() const
{
    return d->m_fetchErrorCode != Syndication::Success;
}

Syndication::ErrorCode Feed::fetchErrorCode() const
{
    return d->m_fetchErrorCode;
}

bool Feed::isArticlesLoaded() const
{
    return d->m_articlesLoaded;
}

QDomElement Feed::toOPML(QDomElement parent, QDomDocument document) const
{
    QDomElement el = document.createElement(QStringLiteral("outline"));
    el.setAttribute(QStringLiteral("text"), title());
    el.setAttribute(QStringLiteral("title"), title());
    el.setAttribute(QStringLiteral("xmlUrl"), d->m_xmlUrl);
    el.setAttribute(QStringLiteral("htmlUrl"), d->m_htmlUrl);
    el.setAttribute(QStringLiteral("id"), QString::number(id()));
    el.setAttribute(QStringLiteral("description"), d->m_description);
    el.setAttribute(QStringLiteral("useCustomFetchInterval"), (useCustomFetchInterval() ? QStringLiteral("true") : QStringLiteral("false")));
    el.setAttribute(QStringLiteral("fetchInterval"), QString::number(fetchInterval()));
    el.setAttribute(QStringLiteral("archiveMode"), archiveModeToString(d->m_archiveMode));
    el.setAttribute(QStringLiteral("maxArticleAge"), d->m_maxArticleAge);
    el.setAttribute(QStringLiteral("comment"), d->m_comment);
    el.setAttribute(QStringLiteral("maxArticleNumber"), d->m_maxArticleNumber);
    if (d->m_markImmediatelyAsRead) {
        el.setAttribute(QStringLiteral("markImmediatelyAsRead"), QStringLiteral("true"));
    }
    if (d->m_useNotification) {
        el.setAttribute(QStringLiteral("useNotification"), QStringLiteral("true"));
    }
    if (d->m_loadLinkedWebsite) {
        el.setAttribute(QStringLiteral("loadLinkedWebsite"), QStringLiteral("true"));
    }
    if (!d->m_faviconInfo.imageUrl.isEmpty()) {
        el.setAttribute(QStringLiteral("faviconUrl"), d->m_faviconInfo.imageUrl);
        if (d->m_faviconInfo.width != -1) {
            el.setAttribute(QStringLiteral("faviconWidth"), d->m_faviconInfo.width);
        }
        if (d->m_faviconInfo.height != -1) {
            el.setAttribute(QStringLiteral("faviconHeight"), d->m_faviconInfo.height);
        }
    }
    if (!d->m_logoInfo.imageUrl.isEmpty()) {
        el.setAttribute(QStringLiteral("logoUrl"), d->m_logoInfo.imageUrl);
        if (d->m_logoInfo.width != -1) {
            el.setAttribute(QStringLiteral("logoWidth"), d->m_logoInfo.width);
        }
        if (d->m_logoInfo.height != -1) {
            el.setAttribute(QStringLiteral("logoHeight"), d->m_logoInfo.height);
        }
    }
    el.setAttribute(QStringLiteral("maxArticleNumber"), d->m_maxArticleNumber);
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

        const uint lastFetch = d->m_archive->lastFetch().toSecsSinceEpoch();

        const uint now = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();

        if (interval > 0 && (now - lastFetch) >= static_cast<uint>(interval)) {
            queue->addFeed(this);
        }
    }
}

void Feed::slotAddFeedIconListener()
{
    if (d->m_faviconInfo.imageUrl.isEmpty()) {
        loadFavicon(d->m_xmlUrl, true);
    } else {
        loadFavicon(d->m_faviconInfo.imageUrl, false);
    }
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

    QVector<Article> deletedArticles = d->m_deletedArticles;

    for (; it != en; ++it) {
        if (!d->articles.contains((*it)->id())) { // article not in list
            Article mya(*it, this);
            mya.offsetPubDate(nudge);
            nudge--;
            appendArticle(mya);
            d->m_addedArticlesNotify.append(mya);

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

                d->m_updatedArticlesNotify.append(mya);
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
        d->m_archive->deleteArticle((*dtmp).guid());
        d->m_removedArticlesNotify.append(*dtmp);
        changed = true;
        d->m_deletedArticles.removeAll(*dtmp);
    }

    if (changed) {
        articlesModified();
    }
}

bool Feed::usesExpiryByAge() const
{
    return (d->m_archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleAge) || d->m_archiveMode == limitArticleAge;
}

bool Feed::isExpired(const Article &a) const
{
    const QDateTime now = QDateTime::currentDateTime();
    int expiryAge = -1;
// check whether the feed uses the global default and the default is limitArticleAge
    if (d->m_archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleAge) {
        expiryAge = Settings::maxArticleAge() * 24 * 3600;
    } else // otherwise check if this feed has limitArticleAge set
    if (d->m_archiveMode == limitArticleAge) {
        expiryAge = d->m_maxArticleAge * 24 * 3600;
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
    d->m_followDiscovery = followDiscovery;
    d->m_fetchTries = 0;

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
    if (d->m_loader) {
        d->m_loader->abort();
    }
}

void Feed::tryFetch()
{
    d->m_fetchErrorCode = Syndication::Success;

    d->m_loader = Syndication::Loader::create(this, SLOT(fetchCompleted(Syndication::Loader*,
                                                                        Syndication::FeedPtr,
                                                                        Syndication::ErrorCode)));
    d->m_loader->loadFrom(QUrl(d->m_xmlUrl), new FeedRetriever());
}

void Feed::fetchCompleted(Syndication::Loader *l, Syndication::FeedPtr doc, Syndication::ErrorCode status)
{
    // Note that loader instances delete themselves
    d->m_loader = nullptr;

    // fetching wasn't successful:
    if (status != Syndication::Success) {
        if (status == Syndication::Aborted) {
            d->m_fetchErrorCode = Syndication::Success;
            Q_EMIT fetchAborted(this);
        } else if (d->m_followDiscovery && (status == Syndication::InvalidXml) && (d->m_fetchTries < 3) && (l->discoveredFeedURL().isValid())) {
            d->m_fetchTries++;
            d->m_xmlUrl = l->discoveredFeedURL().url();
            Q_EMIT fetchDiscovery(this);
            tryFetch();
        } else {
            d->m_fetchErrorCode = status;
            Q_EMIT fetchError(this);
        }
        markAsFetchedNow();
        return;
    }

    loadArticles(); // TODO: make me fly: make this delayed

#if SYNDICATION_VERSION >= QT_VERSION_CHECK(5, 65, 0)
    if (!doc->icon().isNull() && !doc->icon()->url().isEmpty()) {
        loadFavicon(doc->icon()->url(), false);
        d->m_faviconInfo.width = doc->icon()->width();
        d->m_faviconInfo.height = doc->icon()->height();
    } else {
        loadFavicon(xmlUrl(), true);
    }
#else
    loadFavicon(xmlUrl(), true);
#endif
    d->m_fetchErrorCode = Syndication::Success;

    if (!doc->image().isNull()) {
        d->m_logoInfo.imageUrl = doc->image()->url();
        d->m_logoInfo.width = doc->image()->width();
        d->m_logoInfo.height = doc->image()->height();
    }

    if (title().isEmpty()) {
        setTitle(Syndication::htmlToPlainText(doc->title()));
    }

    d->m_description = doc->description();
    d->m_htmlUrl = doc->link();

    appendArticles(doc);

    markAsFetchedNow();
    Q_EMIT fetched(this);
}

void Feed::markAsFetchedNow()
{
    if (d->m_archive) {
        d->m_archive->setLastFetch(QDateTime::currentDateTimeUtc());
    }
}

QIcon Feed::icon() const
{
    if (fetchErrorOccurred()) {
        return QIcon::fromTheme(QStringLiteral("dialog-error"));
    }

    return !d->m_favicon.isNull() ? d->m_favicon : QIcon::fromTheme(QStringLiteral("text-html"));
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
    d->m_favicon = icon;
    nodeModified();
}

void Feed::setLogoInfo(const ImageInfo &image)
{
    if (d->m_logoInfo != image) {
        d->m_logoInfo = image;
        nodeModified();
    }
}

Feed::ArchiveMode Feed::archiveMode() const
{
    return d->m_archiveMode;
}

void Feed::setArchiveMode(ArchiveMode archiveMode)
{
    d->m_archiveMode = archiveMode;
}

int Feed::unread() const
{
    return d->m_archive ? d->m_archive->unread() : 0;
}

void Feed::setUnread(int unread)
{
    if (d->m_archive && unread != d->m_archive->unread()) {
        d->m_archive->setUnread(unread);
        nodeModified();
    }
}

void Feed::setArticleDeleted(Article &a)
{
    d->setTotalCountDirty();
    if (!d->m_deletedArticles.contains(a)) {
        d->m_deletedArticles.append(a);
    }

    d->m_updatedArticlesNotify.append(a);
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
    d->m_updatedArticlesNotify.append(a);
    if (process) {
        articlesModified();
    }
}

int Feed::totalCount() const
{
    if (d->m_totalCount == -1) {
        d->m_totalCount = std::count_if(d->articles.constBegin(), d->articles.constEnd(), [](const Article &art) -> bool {
            return !art.isDeleted();
        });
    }
    return d->m_totalCount;
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
    if (!d->m_addedArticlesNotify.isEmpty()) {
        // copy list, otherwise the refcounting in Article::Private breaks for
        // some reason (causing segfaults)
        const QVector<Article> l = d->m_addedArticlesNotify;
        Q_EMIT signalArticlesAdded(this, l);
        d->m_addedArticlesNotify.clear();
    }
    if (!d->m_updatedArticlesNotify.isEmpty()) {
        // copy list, otherwise the refcounting in Article::Private breaks for
        // some reason (causing segfaults)
        const QVector<Article> l = d->m_updatedArticlesNotify;
        Q_EMIT signalArticlesUpdated(this, l);
        d->m_updatedArticlesNotify.clear();
    }
    if (!d->m_removedArticlesNotify.isEmpty()) {
        // copy list, otherwise the refcounting in Article::Private breaks for
        // some reason (causing segfaults)
        const QVector<Article> l = d->m_removedArticlesNotify;
        Q_EMIT signalArticlesRemoved(this, l);
        d->m_removedArticlesNotify.clear();
    }
    TreeNode::doArticleNotification();
}

void Feed::enforceLimitArticleNumber()
{
    int limit = -1;
    if (d->m_archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleNumber) {
        limit = Settings::maxArticleNumber();
    } else if (d->m_archiveMode == limitArticleNumber) {
        limit = maxArticleNumber();
    }

    if (limit == -1 || limit >= d->articles.count() - d->m_deletedArticles.count()) {
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

bool Feed::ImageInfo::operator==(const Feed::ImageInfo &other) const
{
    return other.width == width
           && other.height == height
           && other.imageUrl == imageUrl;
}

bool Feed::ImageInfo::operator!=(const Feed::ImageInfo &other) const
{
    return !ImageInfo::operator==(other);
}
