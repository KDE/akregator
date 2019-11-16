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

#ifndef AKREGATOR_FEED_H
#define AKREGATOR_FEED_H

#include "akregator_export.h"
#include "treenode.h"

#include <Syndication/Syndication>

#include <QIcon>

class QDomElement;
class QString;

namespace Akregator {
class Article;
class FetchQueue;
class TreeNodeVisitor;
class ArticleDeleteJob;

namespace Backend {
class Storage;
}

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
    static ArchiveMode stringToArchiveMode(const QString &str);

    /** converts ArchiveMode values to corresponding strings */
    static QString archiveModeToString(ArchiveMode mode);

    /** creates a Feed object from a description in OPML format */
    static Feed *fromOPML(const QDomElement &e, Akregator::Backend::Storage *storage);

    /** default constructor */
    explicit Feed(Akregator::Backend::Storage *storage);
    ~Feed();

    Q_REQUIRED_RESULT bool accept(TreeNodeVisitor *visitor) override;

    /** exports the feed settings to OPML */
    Q_REQUIRED_RESULT QDomElement toOPML(QDomElement parent, QDomDocument document) const override;

    /**
        returns whether this feed uses its own fetch interval or the global setting
        @return @c true iff this feed has a custom fetch interval
        */
    Q_REQUIRED_RESULT bool useCustomFetchInterval() const;

    /** set if the feed has its custom fetch interval or uses the
        global setting
        @param enabled @c true: use custom interval, @c false: use global default
        */
    void setCustomFetchIntervalEnabled(bool enabled);

    // FIXME is it -1 or 0 to disable interval fetching?
    /** Returns custom auto fetch interval of this feed.
    @return custom fetch interval in minutes, 0 if disabled */
    Q_REQUIRED_RESULT int fetchInterval() const;

    /** Sets custom auto fetch interval.
    @param interval interval in minutes, -1 for disabling auto fetching */
    void setFetchInterval(int interval);

    /** returns the archiving mode which is used for this feed */
    Q_REQUIRED_RESULT ArchiveMode archiveMode() const;

    /** sets the archiving mode for this feed */
    void setArchiveMode(ArchiveMode archiveMode);

    /** returns the maximum age of articles used for expiration by age (used in @c limitArticleAge archive mode)
    @return expiry age in days */
    Q_REQUIRED_RESULT int maxArticleAge() const;

    /** sets the maximum age of articles used for expiration by age (used in @c limitArticleAge archive mode)
    @param maxArticleAge expiry age in days */
    void setMaxArticleAge(int maxArticleAge);

    /** returns the article count limit used in @c limitArticleNumber archive mode **/
    Q_REQUIRED_RESULT int maxArticleNumber() const;

    /** sets the article count limit used in @c limitArticleNumber archive mode **/
    void setMaxArticleNumber(int maxArticleNumber);

    /** if @c true, new articles are marked immediately as read instead of new/unread. Useful for high-traffic feeds. */
    Q_REQUIRED_RESULT bool markImmediatelyAsRead() const;

    void setMarkImmediatelyAsRead(bool enabled);

    void setUseNotification(bool enabled);

    Q_REQUIRED_RESULT bool useNotification() const;

    /** if true, the linked URL is loaded directly in the article viewer instead of showing the description */
    void setLoadLinkedWebsite(bool enabled);

    Q_REQUIRED_RESULT bool loadLinkedWebsite() const;

    /** returns the feed logo */
    Q_REQUIRED_RESULT Feed::ImageInfo logoInfo() const;

    /** sets the feed image */
    void setLogoInfo(const ImageInfo &image);

    /** returns the url of the actual feed source (rss/rdf/atom file) */
    Q_REQUIRED_RESULT QString xmlUrl() const;
    /** sets the url of the actual feed source (rss/rdf/atom file) */
    void setXmlUrl(const QString &s);

    /** returns the URL of the HTML page of this feed */
    Q_REQUIRED_RESULT QString htmlUrl() const;
    /** sets the URL of the HTML page of this feed */
    void setHtmlUrl(const QString &s);

    Q_REQUIRED_RESULT Feed::ImageInfo faviconInfo() const;
    void setFaviconInfo(const Feed::ImageInfo &url);

    /** returns the description of this feed */
    Q_REQUIRED_RESULT QString description() const;

    /** sets the description of this feed */
    void setDescription(const QString &s);

    /** returns article by guid
        * @param guid the guid of the article to be returned
        * @return the article object with the given guid, or a
        * null article if non-existent
        */
    Q_REQUIRED_RESULT Article findArticle(const QString &guid) const;

    /** returns whether a fetch error has occurred */
    Q_REQUIRED_RESULT bool fetchErrorOccurred() const;

    Syndication::ErrorCode fetchErrorCode() const;

    /** returns the unread count for this feed */
    Q_REQUIRED_RESULT int unread() const override;

    /** returns the number of total articles in this feed
    @return number of articles */

    Q_REQUIRED_RESULT int totalCount() const override;

    /** returns if the article archive of this feed is loaded */
    Q_REQUIRED_RESULT bool isArticlesLoaded() const;

    /** returns if this node is a feed group (@c false here) */
    Q_REQUIRED_RESULT bool isGroup() const override
    {
        return false;
    }

    //impl
    Q_REQUIRED_RESULT bool isAggregation() const override
    {
        return false;
    }

    /** returns the next node in the tree.
    Calling next() unless it returns 0 iterates through the tree in pre-order
    */
    const TreeNode *next() const override;
    TreeNode *next() override;

    //impl
    Q_REQUIRED_RESULT QIcon icon() const override;

    /** deletes expired articles */
    void deleteExpiredArticles(Akregator::ArticleDeleteJob *job);

    Q_REQUIRED_RESULT bool isFetching() const;

    QVector<const Feed *> feeds() const override;
    QVector<Feed *> feeds() override;
    QVector<const Folder *> folders() const override;
    QVector<Folder *> folders() override;

    KJob *createMarkAsReadJob() override;

    Q_REQUIRED_RESULT QString comment() const;
    void setComment(const QString &comment);
    void setFaviconUrl(const QString &url);
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

private:
    void setFavicon(const QIcon &icon);
    void loadFavicon(const QString &url, bool downloadFavicon);
    QVector<Article> articles() override;

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
    bool isExpired(const Article &a) const;

    /** returns @c true if either this article uses @c limitArticleAge as custom setting or uses the global default, which is @c limitArticleAge */
    bool usesExpiryByAge() const;

    /** executes the actual fetch action */
    void tryFetch();

    void markAsFetchedNow();

private Q_SLOTS:

    void fetchCompleted(Syndication::Loader *loader, Syndication::FeedPtr doc, Syndication::ErrorCode errorCode);

private:

    class Private;
    Private *d;
};
} // namespace Akregator

#endif // AKREGATOR_FEED_H
