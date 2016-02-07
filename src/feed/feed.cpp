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

using Syndication::ItemPtr;
using namespace Akregator;

template<typename Key, typename Value, template<typename, typename> class Container>
QVector<Value> valuesToVector(const Container<Key, Value> &container)
{
    QVector<Value> values;
    values.reserve(container.size());
    foreach (const Value &value, container) {
        values << value;
    }
    return values;
}

class Q_DECL_HIDDEN Akregator::Feed::Private
{
    Akregator::Feed *const q;
public:
    explicit Private(Backend::Storage *storage, Akregator::Feed *qq);

    Backend::Storage *storage;
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
    Syndication::Loader *loader;
    bool articlesLoaded;
    Backend::FeedStorage *archive;

    QString xmlUrl;
    QString htmlUrl;
    QString description;

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

QString Akregator::Feed::archiveModeToString(ArchiveMode mode)
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

Akregator::Feed *Akregator::Feed::fromOPML(QDomElement e, Backend::Storage *storage)
{

    if (!e.hasAttribute(QStringLiteral("xmlUrl")) && !e.hasAttribute(QStringLiteral("xmlurl")) && !e.hasAttribute(QStringLiteral("xmlURL"))) {
        return 0;
    }

    QString title = e.hasAttribute(QStringLiteral("text")) ? e.attribute(QStringLiteral("text")) : e.attribute(QStringLiteral("title"));

    QString xmlUrl = e.hasAttribute(QStringLiteral("xmlUrl")) ? e.attribute(QStringLiteral("xmlUrl")) : e.attribute(QStringLiteral("xmlurl"));
    if (xmlUrl.isEmpty()) {
        xmlUrl = e.attribute(QStringLiteral("xmlURL"));
    }

    bool useCustomFetchInterval = e.attribute(QStringLiteral("useCustomFetchInterval")) == QLatin1String("true");

    QString htmlUrl = e.attribute(QStringLiteral("htmlUrl"));
    QString description = e.attribute(QStringLiteral("description"));
    int fetchInterval = e.attribute(QStringLiteral("fetchInterval")).toInt();
    ArchiveMode archiveMode = stringToArchiveMode(e.attribute(QStringLiteral("archiveMode")));
    int maxArticleAge = e.attribute(QStringLiteral("maxArticleAge")).toUInt();
    int maxArticleNumber = e.attribute(QStringLiteral("maxArticleNumber")).toUInt();
    bool markImmediatelyAsRead = e.attribute(QStringLiteral("markImmediatelyAsRead")) == QLatin1String("true");
    bool useNotification = e.attribute(QStringLiteral("useNotification")) == QLatin1String("true");
    bool loadLinkedWebsite = e.attribute(QStringLiteral("loadLinkedWebsite")) == QLatin1String("true");
    uint id = e.attribute(QStringLiteral("id")).toUInt();

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
    feed->loadArticles(); // TODO: make me fly: make this delayed

    return feed;
}

bool Akregator::Feed::accept(TreeNodeVisitor *visitor)
{
    if (visitor->visitFeed(this)) {
        return true;
    } else {
        return visitor->visitTreeNode(this);
    }
}

QVector<const Folder *> Akregator::Feed::folders() const
{
    return QVector<const Folder *>();
}

QVector<Folder *> Akregator::Feed::folders()
{
    return QVector<Folder *>();
}

QVector<const Akregator::Feed *> Akregator::Feed::feeds() const
{
    QVector<const Akregator::Feed *> list;
    list.append(this);
    return list;
}

QVector<Akregator::Feed *> Akregator::Feed::feeds()
{
    QVector<Feed *> list;
    list.append(this);
    return list;
}

Article Akregator::Feed::findArticle(const QString &guid) const
{
    return d->articles.value(guid);
}

QVector<Article> Akregator::Feed::articles()
{
    if (!d->articlesLoaded) {
        loadArticles();
    }
    return valuesToVector(d->articles);
}

Backend::Storage *Akregator::Feed::storage()
{
    return d->storage;
}

void Akregator::Feed::loadArticles()
{
    if (d->articlesLoaded) {
        return;
    }

    if (!d->archive && d->storage) {
        d->archive = d->storage->archiveFor(xmlUrl());
    }

    QStringList list = d->archive->articles();
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        Article mya(*it, this);
        d->articles[mya.guid()] = mya;
        if (mya.isDeleted()) {
            d->deletedArticles.append(mya);
        }
    }

    d->articlesLoaded = true;
    enforceLimitArticleNumber();
    recalcUnreadCount();
}

void Akregator::Feed::recalcUnreadCount()
{
    QVector<Article> tarticles = articles();
    QVector<Article>::ConstIterator it;
    QVector<Article>::ConstIterator en = tarticles.constEnd();

    int oldUnread = d->archive->unread();

    int unread = 0;

    for (it = tarticles.constBegin(); it != en; ++it)
        if (!(*it).isDeleted() && (*it).status() != Read) {
            ++unread;
        }

    if (unread != oldUnread) {
        d->archive->setUnread(unread);
        nodeModified();
    }
}

Akregator::Feed::ArchiveMode Akregator::Feed::stringToArchiveMode(const QString &str)
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

Akregator::Feed::Private::Private(Backend::Storage *storage_, Akregator::Feed *qq)
    : q(qq),
      storage(storage_),
      autoFetch(false),
      fetchInterval(30),
      archiveMode(globalDefault),
      maxArticleAge(60),
      maxArticleNumber(1000),
      markImmediatelyAsRead(false),
      useNotification(false),
      loadLinkedWebsite(false),
      lastFetched(0),
      fetchErrorCode(Syndication::Success),
      fetchTries(0),
      followDiscovery(false),
      loader(0),
      articlesLoaded(false),
      archive(0),
      totalCount(-1)
{
    Q_ASSERT(q);
    Q_ASSERT(storage);
}

Akregator::Feed::Feed(Backend::Storage *storage) : TreeNode(), d(new Private(storage, this))
{
}

Akregator::Feed::~Feed()
{
    slotAbortFetch();
    emitSignalDestroyed();
    delete d;
    d = 0;
}

bool Akregator::Feed::useCustomFetchInterval() const
{
    return d->autoFetch;
}

void Akregator::Feed::setCustomFetchIntervalEnabled(bool enabled)
{
    d->autoFetch = enabled;
}

int Akregator::Feed::fetchInterval() const
{
    return d->fetchInterval;
}

void Akregator::Feed::setFetchInterval(int interval)
{
    d->fetchInterval = interval;
}

int Akregator::Feed::maxArticleAge() const
{
    return d->maxArticleAge;
}

void Akregator::Feed::setMaxArticleAge(int maxArticleAge)
{
    d->maxArticleAge = maxArticleAge;
}

int Akregator::Feed::maxArticleNumber() const
{
    return d->maxArticleNumber;
}

void Akregator::Feed::setMaxArticleNumber(int maxArticleNumber)
{
    d->maxArticleNumber = maxArticleNumber;
}

bool Akregator::Feed::markImmediatelyAsRead() const
{
    return d->markImmediatelyAsRead;
}

bool Akregator::Feed::isFetching() const
{
    return d->loader != 0;
}

void Akregator::Feed::setMarkImmediatelyAsRead(bool enabled)
{
    d->markImmediatelyAsRead = enabled;
    if (enabled) {
        createMarkAsReadJob()->start();
    }
}

void Akregator::Feed::setUseNotification(bool enabled)
{
    d->useNotification = enabled;
}

bool Akregator::Feed::useNotification() const
{
    return d->useNotification;
}

void Akregator::Feed::setLoadLinkedWebsite(bool enabled)
{
    d->loadLinkedWebsite = enabled;
}

bool Akregator::Feed::loadLinkedWebsite() const
{
    return d->loadLinkedWebsite;
}

QPixmap Akregator::Feed::image() const
{
    return d->imagePixmap;
}

QString Akregator::Feed::xmlUrl() const
{
    return d->xmlUrl;
}

void Akregator::Feed::setXmlUrl(const QString &s)
{
    d->xmlUrl = s;
    if (! Settings::fetchOnStartup()) {
        QTimer::singleShot(KRandom::random() % 4000, this, &Feed::slotAddFeedIconListener);    // TODO: let's give a gui some time to show up before starting the fetch when no fetch on startup is used. replace this with something proper later...
    }
}

QString Akregator::Feed::htmlUrl() const
{
    return d->htmlUrl;
}

void Akregator::Feed::setHtmlUrl(const QString &s)
{
    d->htmlUrl = s;
}

QString Akregator::Feed::description() const
{
    return d->description;
}

void Akregator::Feed::setDescription(const QString &s)
{
    d->description = s;
}

bool Akregator::Feed::fetchErrorOccurred() const
{
    return d->fetchErrorCode != Syndication::Success;
}

Syndication::ErrorCode Akregator::Feed::fetchErrorCode() const
{
    return d->fetchErrorCode;
}

bool Akregator::Feed::isArticlesLoaded() const
{
    return d->articlesLoaded;
}

QDomElement Akregator::Feed::toOPML(QDomElement parent, QDomDocument document) const
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

KJob *Akregator::Feed::createMarkAsReadJob()
{
    ArticleModifyJob *job = new ArticleModifyJob;
    Q_FOREACH (const Article &i, articles()) {
        const ArticleId aid = { xmlUrl(), i.guid() };
        job->setStatus(aid, Read);
    }
    return job;
}

void Akregator::Feed::slotAddToFetchQueue(FetchQueue *queue, bool intervalFetchOnly)
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

        uint lastFetch = d->archive->lastFetch();

        uint now = QDateTime::currentDateTime().toTime_t();

        if (interval > 0 && now - lastFetch >= (uint)interval) {
            queue->addFeed(this);
        }
    }
}

void Akregator::Feed::slotAddFeedIconListener()
{
    //FeedIconManager::self()->addListener(QUrl(d->xmlUrl), this);
}

void Akregator::Feed::appendArticles(const Syndication::FeedPtr &feed)
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

bool Akregator::Feed::usesExpiryByAge() const
{
    return (d->archiveMode == globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::limitArticleAge) || d->archiveMode == limitArticleAge;
}

bool Akregator::Feed::isExpired(const Article &a) const
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

    return (expiryAge != -1 && a.pubDate().secsTo(now) > expiryAge);
}

void Akregator::Feed::appendArticle(const Article &a)
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

void Akregator::Feed::fetch(bool followDiscovery)
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

void Akregator::Feed::slotAbortFetch()
{
    if (d->loader) {
        d->loader->abort();
    }
}

void Akregator::Feed::tryFetch()
{
    d->fetchErrorCode = Syndication::Success;

    d->loader = Syndication::Loader::create(this, SLOT(fetchCompleted(Syndication::Loader *,
                                            Syndication::FeedPtr,
                                            Syndication::ErrorCode)));
    //connect(d->loader, SIGNAL(progress(ulong)), this, SLOT(slotSetProgress(ulong)));
    d->loader->loadFrom(d->xmlUrl);
}

void Akregator::Feed::slotImageFetched(const QPixmap &image)
{
    setImage(image);
}

void Akregator::Feed::fetchCompleted(Syndication::Loader *l, Syndication::FeedPtr doc, Syndication::ErrorCode status)
{
    // Note that loader instances delete themselves
    d->loader = 0;

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

    //FeedIconManager::self()->addListener(QUrl(xmlUrl()), this);

    d->fetchErrorCode = Syndication::Success;

    if (d->imagePixmap.isNull()) {
        //QString u = d->xmlUrl;
        QString imageFileName = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/') + QLatin1String("akregator/Media/") + Utils::fileNameForUrl(d->xmlUrl) + QLatin1String(".png");
        d->imagePixmap = QPixmap(imageFileName, "PNG");

        // if we ain't got the image and the feed provides one, get it....
        // TODO: reenable image fetching!
        if (false) { // d->imagePixmap.isNull() && doc.image())
            //d->image = *doc.image();
            //connect(&d->image, SIGNAL(gotPixmap(QPixmap)), this, SLOT(slotImageFetched(QPixmap)));
            //d->image.getPixmap();
        }
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

void Akregator::Feed::markAsFetchedNow()
{
    if (d->archive) {
        d->archive->setLastFetch(QDateTime::currentDateTime().toTime_t());
    }
}

QIcon Akregator::Feed::icon() const
{
    if (fetchErrorOccurred()) {
        return QIcon::fromTheme(QStringLiteral("dialog-error"));
    }

    return !d->favicon.isNull() ? d->favicon : QIcon::fromTheme(QStringLiteral("text-html"));
}

void Akregator::Feed::deleteExpiredArticles(ArticleDeleteJob *deleteJob)
{
    if (!usesExpiryByAge()) {
        return;
    }

    setNotificationMode(false);

    QList<ArticleId> toDelete;
    const QString feedUrl = xmlUrl();
    const bool useKeep = Settings::doNotExpireImportantArticles();

    Q_FOREACH (const Article &i, d->articles) {
        if ((!useKeep || !i.keep()) && isExpired(i)) {
            const ArticleId aid = { feedUrl, i.guid() };
            toDelete.append(aid);
        }
    }

    deleteJob->appendArticleIds(toDelete);
    setNotificationMode(true);
}
#if 0
void Akregator::Feed::setFavicon(const QIcon &icon)
{
    d->favicon = icon;
    nodeModified();
}
#endif
void Akregator::Feed::setImage(const QPixmap &p)
{
    if (p.isNull()) {
        return;
    }
    d->imagePixmap = p;
    const QString filename = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/') + QString(QLatin1String("akregator/Media/") + Utils::fileNameForUrl(d->xmlUrl) + QLatin1String(".png"));
    QFileInfo fileInfo(filename);
    QDir().mkpath(fileInfo.absolutePath());
    d->imagePixmap.save(filename, "PNG");
    nodeModified();
}

Akregator::Feed::ArchiveMode Akregator::Feed::archiveMode() const
{
    return d->archiveMode;
}

void Akregator::Feed::setArchiveMode(ArchiveMode archiveMode)
{
    d->archiveMode = archiveMode;
}

int Akregator::Feed::unread() const
{
    return d->archive ? d->archive->unread() : 0;
}

void Akregator::Feed::setUnread(int unread)
{
    if (d->archive && unread != d->archive->unread()) {
        d->archive->setUnread(unread);
        nodeModified();
    }
}

void Akregator::Feed::setArticleDeleted(Article &a)
{
    d->setTotalCountDirty();
    if (!d->deletedArticles.contains(a)) {
        d->deletedArticles.append(a);
    }

    d->updatedArticlesNotify.append(a);
    articlesModified();
}

void Akregator::Feed::setArticleChanged(Article &a, int oldStatus)
{
    if (oldStatus != -1) {
        int newStatus = a.status();
        if (oldStatus == Read && newStatus != Read) {
            setUnread(unread() + 1);
        } else if (oldStatus != Read && newStatus == Read) {
            setUnread(unread() - 1);
        }
    }
    d->updatedArticlesNotify.append(a);
    articlesModified();
}

int Akregator::Feed::totalCount() const
{
    if (d->totalCount == -1) {
        d->totalCount = std::count_if(d->articles.constBegin(), d->articles.constEnd(), [](const Article & art) -> bool { return !art.isDeleted(); });
    }
    return d->totalCount;
}

TreeNode *Akregator::Feed::next()
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
    return 0;
}

const TreeNode *Akregator::Feed::next() const
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
    return 0;
}

void Akregator::Feed::doArticleNotification()
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

void Akregator::Feed::enforceLimitArticleNumber()
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
    qSort(articles);

    int c = 0;
    const bool useKeep = Settings::doNotExpireImportantArticles();

    Q_FOREACH (Article i, articles) { //krazy:exclude=foreach
        if (c < limit) {
            if (!i.isDeleted() && (!useKeep || !i.keep())) {
                ++c;
            }
        } else if (!useKeep || !i.keep()) {
            i.setDeleted();
        }
    }
}

