/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORFEED_H
#define AKREGATORFEED_H

#include <qpixmap.h>
#include <qstring.h>
#include <kurl.h>

#include "articlesequence.h"
#include "treenode.h"
#include "librss/librss.h"

using namespace RSS;

class QDomElement;

namespace Akregator
{
    class FetchTransaction;
    class FeedGroup;
    
    /**
        represents a feed
     */
    class Feed : public TreeNode
    {
        Q_OBJECT
        public:
            /** the archiving modes:
                - globalDefault: use default from Settings (default)
                - keepAllArticles: Don't delete any articles
                - disableArchiving: Don't save any articles except articles with keep flag set (equal to maxArticleNumber() == 0)
                - limitArticleNumber: Save maxArticleNumber() articles, plus the ones with keep flag set
                - limitArticleAge: Save articles not older than maxArticleAge() (or keep flag set)
             */
            enum ArchiveMode { globalDefault, keepAllArticles, disableArchiving, limitArticleNumber, limitArticleAge };

            // class methods
            /** converts strings to ArchiveMode value
             if parsing fails, it returns ArchiveMode::globalDefault
             */
            static ArchiveMode stringToArchiveMode(const QString& str);

            /** converts ArchiveMode values to corresponding strings */
            static QString archiveModeToString(ArchiveMode mode);

            /** creates a Feed object from a description in OPML format */
            static Feed* fromOPML(QDomElement e);

            /** default constructor */
            Feed();

            ~Feed();
            
            /** exports the feed settings to OPML */
            virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const;

            /** exports the articles of this feed as XML (for the archive) */
            void dumpXmlData( QDomElement parent, QDomDocument document );

            /**
              returns whether this feed uses its own fetch interval or the global setting
              @return @c true iff this feed has a custom fetch interval
             */
            bool useCustomFetchInterval() const { return m_autoFetch; }

            /** set if the feed has its custom fetch interval or uses the
                global setting
                @param enabled @c true: use custom interval, @c false: use global default
             */
            void setCustomFetchIntervalEnabled(bool enabled) { m_autoFetch = enabled; }

            // FIXME is it -1 or 0 to disable interval fetching?
            /** Returns custom auto fetch interval of this feed.
            @return custom fetch interval in minutes, 0 if disabled */
            int fetchInterval() const { return m_fetchInterval; }

            /** Sets custom auto fetch interval.
            @param interval interval in minutes, -1 for disabling auto fetching */
            void setFetchInterval(int interval) { m_fetchInterval = interval; }

            /** returns the archiving mode which is used for this feed */
            ArchiveMode archiveMode() const;
            
            /** sets the archiving mode for this feed */
            void setArchiveMode(ArchiveMode archiveMode);

            /** returns the maximum age of articles used for expiration by age (used in @c limitArticleAge archive mode)
            @return expiry age in days */
            int maxArticleAge() const { return m_maxArticleAge; }

            /** sets the maximum age of articles used for expiration by age (used in @c limitArticleAge archive mode)
            @param maxArticleAge expiry age in days */
            void setMaxArticleAge(int maxArticleAge)
            { m_maxArticleAge = maxArticleAge; }

            /** returns the article count limit used in @c limitArticleNumber archive mode **/
            int maxArticleNumber() const { return m_maxArticleNumber; }
            
            /** sets the article count limit used in @c limitArticleNumber archive mode **/
            void setMaxArticleNumber(int maxArticleNumber) { m_maxArticleNumber = maxArticleNumber; }

            /** returns the favicon */
            const QPixmap& favicon() const { return m_favicon; }
            
            /** sets the favicon (used in the tree view) */
            void setFavicon(const QPixmap& p);

            /** returns the feed image */
            const QPixmap& image() const { return m_image; }
            
            /** sets the feed image */
            void setImage(const QPixmap &p); 

            /** returns the url of the actual feed source (rss/rdf/atom file) */
            const QString& xmlUrl() const { return m_xmlUrl; }
            /** sets the url of the actual feed source (rss/rdf/atom file) */
            void setXmlUrl(const QString& s) { m_xmlUrl = s; }

            /** returns the URL of the HTML page of this feed */
            const QString& htmlUrl() const { return m_htmlUrl; }
            /** sets the URL of the HTML page of this feed */
            void setHtmlUrl(const QString& s) { m_htmlUrl = s; }

            /** returns the description of this feed */
            const QString& description() const { return m_description; }
            
            /** sets the description of this feed */
            void setDescription(const QString& s) { m_description = s; }

            /** returns the feed's articles, sorted by date */
            virtual ArticleSequence articles();

            /** appends the articles in @c document to the feed's article list
             */
            void appendArticles(const Document &d);

            /** abort fetching */
            void abortFetch();

            /** returns whether a fetch error has occured */
            bool fetchErrorOccurred() { return m_fetchError; }

            /** returns the unread count for this feed */
            virtual int unread() const { return m_unread; }
            
            /** sets the unread count for this feed */
            void setUnread(int unread);

            /** returns if the article archive of this feed is loaded */
            bool isMerged() const { return m_merged; }

            /** set this to true when after the article archive is loaded (for use in @ref Archive::load())*/
            void setMerged(bool m) { m_merged = m; }

            /** returns if this node is a feed group (@c false here) */
            virtual bool isGroup() const { return false; }
           
            /** returns the next node in the tree.
            Calling next() unless it returns 0 iterates through the tree in pre-order
            */
            virtual TreeNode* next();
            
        public slots:
            /** starts fetching */
            void fetch(bool followDiscovery=false, FetchTransaction *f = 0);

            /** deletes expired articles */
            virtual void slotDeleteExpiredArticles();

            /** mark all articles in this feed as read */
            virtual void slotMarkAllArticlesAsRead();
            
            /** add this feed to the fetch transaction @c transaction */
            virtual void slotAddToFetchTransaction(FetchTransaction* transaction);

        signals:
            /** emitted when fetching started */
            void fetchStarted(Feed*);
            /** emitted when feed finished fetching */
            void fetched(Feed *);
            /** emitted when a fetch error occured */
            void fetchError(Feed *);
            /** emitted when a feed URL was found by auto discovery */
            void fetchDiscovery(Feed *);
            /** emitted when a fetch is aborted */
            void fetchAborted(Feed *);
            /** emitted when the feed image is loaded */
            void imageLoaded(Feed*);


        private slots:
            
            void fetchCompleted(Loader *loader, Document doc, Status status);

            /** downloads the favicon */
            void loadFavicon();

        private:
            /** appends article @c a to the article list */
            void appendArticle(const MyArticle& a);

            /** checks whether article @c a is expired (considering custom and global archive mode settings) */
            bool isExpired(const MyArticle& a) const;
            
            /** returns @c true if either this article uses @c limitArticleAge as custom setting or uses the global default, which is @c limitArticleAge */
            bool usesExpiryByAge() const;

            /** executes the actual fetch action
             */
            void tryFetch();
            
            // attributes:
            
            bool m_autoFetch;
            int m_fetchInterval;
	        ArchiveMode m_archiveMode; 
            int m_maxArticleAge; 
            int m_maxArticleNumber;
            
            Document m_document;            
            FetchTransaction* m_transaction;
            
            bool m_fetchError;
            int m_fetchTries;
            bool m_followDiscovery;
            Loader* m_loader;
            bool m_merged;
            
            /** caches the unread count of the feed's articles */
	        int m_unread;

            /** URL of RSS feed itself */
            QString m_xmlUrl;

            /** URL of HTML page for this feed */
            QString m_htmlUrl;

            /** Verbose feed description. */
            QString m_description;   

            /** list of feed articles, sorted by date */
            ArticleSequence m_articles;     

            QPixmap m_image;
            QPixmap m_favicon;
 
    };
}

#endif
