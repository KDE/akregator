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
#include "librss/librss.h" /* <rss/librss> ! */

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
	
            enum ArchiveMode { globalDefault, keepAllArticles, disableArchiving, limitArticleNumber, limitArticleAge };
                                    
            static ArchiveMode stringToArchiveMode(const QString& str);
            static QString archiveModeToString(ArchiveMode mode);
            static Feed* fromOPML(QDomElement e);
            
            Feed();
            ~Feed();

            virtual QDomElement toOPML( QDomElement parent, QDomDocument document ) const;
            void dumpXmlData( QDomElement parent, QDomDocument document );

            virtual bool isGroup() const { return false; }
            
            bool autoFetch() const { return m_autoFetch; }
            
            void setAutoFetch(bool enable) { m_autoFetch = enable; }

            /** Returns custom auto fetch interval of this feed.
            @return custom fetch interval in minutes, 0 if disabled */
            int fetchInterval() const { return m_fetchInterval; }

            /** Sets custom auto fetch interval.
            @param interval interval in minutes, -1 for disabling auto fetching */
            void setFetchInterval(int interval) { m_fetchInterval = interval; }

            ArchiveMode archiveMode() const { return m_archiveMode; }
            void setArchiveMode(ArchiveMode archiveMode) { m_archiveMode = archiveMode; }  
//             
            int maxArticleAge() const { return m_maxArticleAge; } 
            void setMaxArticleAge(int maxArticleAge)
            { m_maxArticleAge = maxArticleAge; }
            
            int maxArticleNumber() const { return m_maxArticleNumber; } 
            void setMaxArticleNumber(int maxArticleNumber) { m_maxArticleNumber = maxArticleNumber; }
            
            bool isMerged() const { return m_merged; }
            void setMerged(bool m){ m_merged = m;}

            virtual int unread() const { return m_unread; }
            void setUnread(int unread);
            
            const QPixmap& favicon() const { return m_favicon; }
            void setFavicon(const QPixmap& p);
           
            const QPixmap& image() const { return m_image; }
            void setImage(const QPixmap &p); 
            
            const QString& xmlUrl() const { return m_xmlUrl; }
            void setXmlUrl(const QString& s) { m_xmlUrl = s; }
            
            const QString& htmlUrl() const { return m_htmlUrl; }
            void setHtmlUrl(const QString& s) { m_htmlUrl = s; }
            
            const QString& description() const { return m_description; }
            void setDescription(const QString& s) { m_description = s; }
          
            virtual ArticleSequence articles();
            
            void appendArticles(const Document &d, bool findDups=false);

            void abortFetch();

            bool fetchErrorOccurred() { return m_fetchError; }
           
            /** returns the next node in the tree.
            Calling next() unless it returns 0 iterates through the tree in pre-order
            */
            virtual TreeNode* next();
            
        public slots:
            void fetch(bool follow=false, FetchTransaction *f=0);
            void loadFavicon();
            virtual void slotDeleteExpiredArticles();
            virtual void slotMarkAllArticlesAsRead();
            virtual void slotAddToFetchTransaction(FetchTransaction* transaction);

        signals:
            void fetchStarted(Feed*);
            void fetched(Feed *);         ///< Emitted when feed finishes fetching
            void fetchError(Feed *);
            void fetchDiscovery(Feed *);
            void fetchAborted(Feed *);
            void imageLoaded(Feed*);


        private slots:
            void fetchCompleted(Loader *loader, Document doc, Status status);

        private:
            void appendArticle(const MyArticle& a);
            bool isExpired(const MyArticle& a) const;
            bool usesExpiryByAge() const;
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
