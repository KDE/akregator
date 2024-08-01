/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregator_export.h"
#include "treenode.h"

#include <Syndication/Syndication>

#include <QIcon>

#include <memory>

class QDomElement;
class QString;

namespace Akregator
{
class Article;
class FetchQueue;
class TreeNodeVisitor;
class ArticleDeleteJob;

namespace Backend
{
class Storage;
}

class FeedPrivate;

/** represents a feed */
class AKREGATOR_EXPORT Feed : public TreeNode
{
    friend class ::Akregator::Article;
    friend class ::Akregator::Folder;
    Q_OBJECT
public:
    /** the archiving modes */

    enum ArchiveMode {
        globalDefault, /**< use default from Settings (default) */
        keepAllArticles, /**< Don't delete any articles */
        disableArchiving, /**< Don't save any articles except articles with keep flag set (equal to maxArticleNumber() == 0) */
        limitArticleNumber, /**< Save maxArticleNumber() articles, plus the ones with keep flag set */
        limitArticleAge /**< Save articles not older than maxArticleAge() (or keep flag set) */
    };

    struct ImageInfo {
        QString imageUrl;
        int width = -1;
        int height = -1;
        bool operator==(const ImageInfo &other) const;
        bool operator!=(const ImageInfo &other) const;
    };

    // class methods
    /** converts strings to ArchiveMode value
        if parsing fails, it returns ArchiveMode::globalDefault
        */
    [[nodiscard]] static ArchiveMode stringToArchiveMode(const QString &str);

    /** converts ArchiveMode values to corresponding strings */
    [[nodiscard]] static QString archiveModeToString(ArchiveMode mode);

    /** creates a Feed object from a description in OPML format */
    static Feed *fromOPML(const QDomElement &e, Akregator::Backend::Storage *storage);

    /** default constructor */
    explicit Feed(Akregator::Backend::Storage *storage);
    ~Feed() override;

    [[nodiscard]] bool accept(TreeNodeVisitor *visitor) override;

    /** exports the feed settings to OPML */
    [[nodiscard]] QDomElement toOPML(QDomElement parent, QDomDocument document) const override;

    /**
        returns whether this feed uses its own fetch interval or the global setting
        @return @c true iff this feed has a custom fetch interval
        */
    [[nodiscard]] bool useCustomFetchInterval() const;

    /** set if the feed has its custom fetch interval or uses the
        global setting
        @param enabled @c true: use custom interval, @c false: use global default
        */
    void setCustomFetchIntervalEnabled(bool enabled);

    // FIXME is it -1 or 0 to disable interval fetching?
    /** Returns custom auto fetch interval of this feed.
    @return custom fetch interval in minutes, 0 if disabled */
    [[nodiscard]] int fetchInterval() const;

    /** Sets custom auto fetch interval.
    @param interval interval in minutes, -1 for disabling auto fetching */
    void setFetchInterval(int interval);

    /** returns the archiving mode which is used for this feed */
    [[nodiscard]] ArchiveMode archiveMode() const;

    /** sets the archiving mode for this feed */
    void setArchiveMode(ArchiveMode archiveMode);

    /** returns the maximum age of articles used for expiration by age (used in @c limitArticleAge archive mode)
    @return expiry age in days */
    [[nodiscard]] int maxArticleAge() const;

    /** sets the maximum age of articles used for expiration by age (used in @c limitArticleAge archive mode)
    @param maxArticleAge expiry age in days */
    void setMaxArticleAge(int maxArticleAge);

    /** returns the article count limit used in @c limitArticleNumber archive mode **/
    [[nodiscard]] int maxArticleNumber() const;

    /** sets the article count limit used in @c limitArticleNumber archive mode **/
    void setMaxArticleNumber(int maxArticleNumber);

    /** if @c true, new articles are marked immediately as read instead of new/unread. Useful for high-traffic feeds. */
    [[nodiscard]] bool markImmediatelyAsRead() const;

    void setMarkImmediatelyAsRead(bool enabled);

    void setUseNotification(bool enabled);

    [[nodiscard]] bool useNotification() const;

    /** if true, the linked URL is loaded directly in the article viewer instead of showing the description */
    void setLoadLinkedWebsite(bool enabled);

    [[nodiscard]] bool loadLinkedWebsite() const;

    /** returns the feed logo */
    [[nodiscard]] Feed::ImageInfo logoInfo() const;

    /** sets the feed image */
    void setLogoInfo(const ImageInfo &image);

    /** returns the url of the actual feed source (rss/rdf/atom file) */
    [[nodiscard]] QString xmlUrl() const;
    /** sets the url of the actual feed source (rss/rdf/atom file) */
    void setXmlUrl(const QString &s);

    /** returns the URL of the HTML page of this feed */
    [[nodiscard]] QString htmlUrl() const;
    /** sets the URL of the HTML page of this feed */
    void setHtmlUrl(const QString &s);

    [[nodiscard]] Feed::ImageInfo faviconInfo() const;
    void setFaviconInfo(const Feed::ImageInfo &url);

    /** returns the description of this feed */
    [[nodiscard]] QString description() const;

    /** sets the description of this feed */
    void setDescription(const QString &s);

    /** returns article by guid
     * @param guid the guid of the article to be returned
     * @return the article object with the given guid, or a
     * null article if non-existent
     */
    [[nodiscard]] Article findArticle(const QString &guid) const;

    /** returns whether a fetch error has occurred */
    [[nodiscard]] bool fetchErrorOccurred() const;

    [[nodiscard]] Syndication::ErrorCode fetchErrorCode() const;

    /** returns the unread count for this feed */
    [[nodiscard]] int unread() const override;

    /** returns the number of total articles in this feed
    @return number of articles */

    [[nodiscard]] int totalCount() const override;

    /** returns if the article archive of this feed is loaded */
    [[nodiscard]] bool isArticlesLoaded() const;

    /** returns if this node is a feed group (@c false here) */
    [[nodiscard]] bool isGroup() const override
    {
        return false;
    }

    // impl
    [[nodiscard]] bool isAggregation() const override
    {
        return false;
    }

    /** returns the next node in the tree.
    Calling next() unless it returns 0 iterates through the tree in pre-order
    */
    const TreeNode *next() const override;
    TreeNode *next() override;

    // impl
    [[nodiscard]] QIcon icon() const override;

    /** deletes expired articles */
    void deleteExpiredArticles(Akregator::ArticleDeleteJob *job);

    [[nodiscard]] bool isFetching() const;

    QList<const Feed *> feeds() const override;
    QList<Feed *> feeds() override;
    QList<const Folder *> folders() const override;
    QList<Folder *> folders() override;

    KJob *createMarkAsReadJob() override;

    [[nodiscard]] QString comment() const;
    void setComment(const QString &comment);
    void setFaviconLocalPath(const QString &file);
    void setCopyright(const QString &copyright);
    [[nodiscard]] QString copyright() const;
    [[nodiscard]] QStringList activities() const;
    void setActivities(const QStringList &lst);
public Q_SLOTS:
    /** starts fetching */
    void fetch(bool followDiscovery = false);

    void slotAbortFetch();

    /** add this feed to the fetch queue @c queue */
    void slotAddToFetchQueue(Akregator::FetchQueue *queue, bool intervalFetchOnly = false) override;

    void slotAddFeedIconListener();

Q_SIGNALS:
    /** emitted when fetching started */
    void fetchStarted(Akregator::Feed *);
    /** emitted when feed finished fetching */
    void fetched(Akregator::Feed *);
    /** emitted when a fetch error occurred */
    void fetchError(Akregator::Feed *);
    /** emitted when a feed URL was found by auto discovery */
    void fetchDiscovery(Akregator::Feed *);
    /** emitted when a fetch is aborted */
    void fetchAborted(Akregator::Feed *);

private:
    Akregator::Backend::Storage *storage();
    void setFavicon(const QIcon &icon);
    void loadFavicon(const QString &url, bool downloadFavicon);
    [[nodiscard]] QList<Article> articles() override;

    /** loads articles from archive **/
    void loadArticles();
    void enforceLimitArticleNumber();

    void recalcUnreadCount();

    void doArticleNotification() override;

    /** sets the unread count for this feed */
    void setUnread(int unread);

    /** notifies that article @c mya was set to "deleted".
        To be called by @ref Article
        */
    void setArticleDeleted(Article &a);

    /** Notifies that article @p a was changed.
        @param oldStatus The old status if the status was changed,
        or -1 if the status was not changed
        @param process Set to @c false to disable processing the change
        (updating article list and updating on-screen unread count)
        To be called by @ref Article
        */
    void setArticleChanged(Article &a, int oldStatus = -1, bool process = true);

    void appendArticles(const Syndication::FeedPtr &feed);

    /** appends article @c a to the article list */
    void appendArticle(const Article &a);

    /** checks whether article @c a is expired (considering custom and global archive mode settings) */
    [[nodiscard]] bool isExpired(const Article &a) const;

    /** returns @c true if either this article uses @c limitArticleAge as custom setting or uses the global default, which is @c limitArticleAge */
    [[nodiscard]] bool usesExpiryByAge() const;

    /** executes the actual fetch action */
    void tryFetch();

    void markAsFetchedNow();

private Q_SLOTS:

    void fetchCompleted(Syndication::Loader *loader, Syndication::FeedPtr doc, Syndication::ErrorCode errorCode);

private:
    std::unique_ptr<FeedPrivate> const d;
};
} // namespace Akregator
Q_DECLARE_TYPEINFO(Akregator::Feed::ImageInfo, Q_RELOCATABLE_TYPE);
