/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORFEED_H
#define AKREGATORFEED_H

#include <qpixmap.h>
#include <kurl.h>

#include "feedgroup.h"
#include "librss/librss.h" /* <rss/librss> ! */
#include "myarticle.h"
#include "akregatorconfig.h"

using namespace RSS;

namespace Akregator
{
    class FeedsCollection;
    class FetchTransaction;

    /** This is article list supporting sorting added articles by pubDate
     *  and properly emulating pubDate for articles with invalid pubDates.
     */
    class ArticleSequence : public MyArticle::List
    {
        public:
	
            ArticleSequence();
            ArticleSequence(const ArticleSequence &other);
            virtual ~ArticleSequence();

            iterator insert( iterator it, const MyArticle &x );
            void insert( iterator it, size_type n, const MyArticle &x );
            iterator append( const MyArticle &x );
            iterator prepend( const MyArticle &x );

            void enableSorting(bool b);
            void sort();
        
        private:
            struct Private;
            Private *d;
    };

    class Feed : public FeedGroup
    {
        Q_OBJECT
        public:
	
            enum ArchiveMode { globalDefault, keepAllArticles, disableArchiving, limitArticleNumber, limitArticleAge };
                        
            static ArchiveMode stringToArchiveMode(QString str);
            static QString archiveModeToString(ArchiveMode mode);
            Feed(QListViewItem *i, FeedsCollection *coll);
            ~Feed();

            virtual QDomElement toXml( QDomElement parent, QDomDocument document ) const;
            void dumpXmlData( QDomElement parent, QDomDocument document );

            virtual bool isGroup() const { return false; }
            
            bool autoFetch() const { return m_autoFetch; }
            void setAutoFetch(bool enable) { m_autoFetch = enable; }
            
            int fetchInterval() const { return m_fetchInterval; }
            void setFetchInterval(int interval) { m_fetchInterval = interval; }

            ArchiveMode archiveMode() const { return m_archiveMode; }
            void setArchiveMode(ArchiveMode archiveMode) { m_archiveMode = archiveMode; }  
            
            int maxArticleAge() { return m_maxArticleAge; } const
            void setMaxArticleAge(int maxArticleAge)
            { m_maxArticleAge = maxArticleAge; }
            
            int maxArticleNumber() { return m_maxArticleNumber; } const
            void setMaxArticleNumber(int maxArticleNumber) { m_maxArticleNumber = maxArticleNumber; }
            
            bool isMerged() const { return m_merged; }
            void setMerged(bool m){ m_merged = m;}

            int unread() const { return m_unread; }
            void setUnread(int i) { m_unread = i; }
            
            const QPixmap& favicon() const { return m_favicon; }
            void setFavicon(const QPixmap& p);
           
            const QPixmap& image() const { return m_image; }
            void setImage(const QPixmap &p); 
            
            QString xmlUrl() const { return m_xmlUrl; }
            void setXmlUrl(const QString& s) { m_xmlUrl = s; }
            
            QString htmlUrl() const { return m_htmlUrl; }
            void setHtmlUrl(const QString& s) { m_htmlUrl = s; }
            
            QString description() const { return m_description; }
            void setDescription(const QString& s) { m_description = s; }
          
            const ArticleSequence& articles() const { return m_articles; }
            void markAllRead();

            void appendArticles(const Document &d, bool findDups=false);
            void appendArticle(const MyArticle &a);

            void abortFetch();

           
	    
        public slots:
            void fetch(bool follow=false, FetchTransaction *f=0);
            void loadFavicon();
            void deleteExpiredArticles();

        signals:
            void fetched(Feed *);         ///< Emitted when feed finishes fetching
            void fetchError(Feed *);
            void fetchDiscovery(Feed *);

            void imageLoaded(Feed*);


        private slots:
            void fetchCompleted(Loader *loader, Document doc, Status status);

        private:
            
            void tryFetch();
            
            
            bool m_autoFetch;
            int m_fetchInterval;
	    ArchiveMode m_archiveMode; 
            int m_maxArticleAge; 
            int m_maxArticleNumber;
            Document m_document;            
            bool m_fetchError;
            int m_fetchTries;
            bool m_followDiscovery;
            Loader* m_loader;
            bool m_merged;
	    FetchTransaction* m_transaction;
            int m_unread;
            
            QString m_xmlUrl;        ///< URL of RSS feed itself.
            QString m_htmlUrl;       ///< URL of HTML page for this feed.
            QString m_description;   ///< Verbose feed description.
           
            ArticleSequence m_articles;      ///< List of all feed articles.

            QPixmap m_image;
            QPixmap m_favicon;
 
    };
}

#endif
