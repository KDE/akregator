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

using namespace RSS;

namespace Akregator
{
    class FeedsCollection;

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
/*
            void doNotSort(); ///< Disables internal item sorting (useful for massive insert operations).
            void sort();      ///< Enables internal sorting and sorts items.
*/
        private:
            struct Private;
            Private *d;
    };

    class Feed : public FeedGroup
    {
        Q_OBJECT
        public:
            Feed(QListViewItem *i, FeedsCollection *coll);
            ~Feed();

            virtual QDomElement toXml( QDomElement parent, QDomDocument document );
            void dumpXmlData( QDomElement parent, QDomDocument document );
            
            virtual bool isGroup();

            enum LJAuthMode { AuthNone, AuthLocal, AuthGlobal };

            QString ljAuthModeStr();
            static LJAuthMode authModeFromString(const QString &mode);

            // -- ATTRIBUTES
            //inherited      title();       ///< Feed title
            QString          xmlUrl;        ///< URL of RSS feed itself
            QString          htmlUrl;       ///< URL of HTML page for this feed
            QString          description;   ///< Verbose feed description.
            bool             isLiveJournal; ///< Is this a LiveJournal feed?
            QString          ljUserName;    ///< Name of LJ user whose feed we are fetching.
            LJAuthMode       ljAuthMode;
            QString          ljLogin;       ///< LiveJournal username
            QString          ljPassword;    ///< LiveJournal password md5 digest
            bool             updateTitle;   ///< Whether to update feed title based on fetched rss.
            ArticleSequence  articles;      ///< List of all feed articles

            QPixmap        image;
            QPixmap        favicon;

            bool isMerged(){return m_merged;}
            void setMerged(bool m){m_merged=m;}
           
            int unread(){return m_unread;}
            void setUnread(int i){m_unread=i;}
            void markAllRead();

            void appendArticles(const Document &d, bool findDups=false);
            void appendArticle(const MyArticle &a);
                
            
        public slots:
            void fetch(bool follow=false);                 ///< Start fetching rss
            void loadFavicon();

        signals:
            void fetched(Feed *);         ///< Emitted when feed finishes fetching
            void fetchError(Feed *);
            void fetchDiscovery(Feed *);
            
            void faviconLoaded(Feed*);
            void imageLoaded(Feed*);


        private slots:
            void fetchCompleted(Loader *loader, Document doc, Status status);
            void faviconChanged(const QString &url, const QPixmap &p);
            void imageChanged(const QPixmap &p);

        private:
            void tryFetch();
            bool m_fetchError;
            bool m_followDiscovery;
            int m_fetchTries;
            bool m_merged;
            Document m_document;
            int m_unread;
    };
}

#endif
