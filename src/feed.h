/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef AKREGATORFEED_H
#define AKREGATORFEED_H

#include "feedgroup.h"
#include "librss.h" /* <rss/librss> ! */

using namespace RSS;

namespace Akregator
{
    class FeedsCollection;

    class Feed : public FeedGroup
    {
        Q_OBJECT
        public:
            Feed(QListViewItem *i, FeedsCollection *coll);
            ~Feed();

            virtual QDomElement toXml( QDomElement parent, QDomDocument document );

            virtual bool isGroup();

            enum LJAuthMode { AuthNone, AuthLocal, AuthGlobal };

            QString ljAuthModeStr();
            static LJAuthMode authModeFromString(const QString &mode);

            // -- ATTRIBUTES
            //inherited    title();       ///< Feed title
            QString        xmlUrl;        ///< URL of RSS feed itself
            QString        htmlUrl;       ///< URL of HTML page for this feed
            QString        description;   ///< Verbose feed description.
            bool           isLiveJournal; ///< Is this a LiveJournal feed?
            QString        ljUserName;    ///< Name of LJ user whose feed we are fetching.
            LJAuthMode     ljAuthMode;
            QString        ljLogin;       ///< LiveJournal username
            QString        ljPassword;    ///< LiveJournal password md5 digest
            bool           updateTitle;   ///< Whether to update feed title based on fetched rss.
            Article::List  articles;      ///< List of just fetched feed articles (will be merged with archive?)

            void fetch();                 ///< Start fetching rss

        signals:
            void fetched(Feed *);         ///< Emitted when feed finishes fetching

        private slots:
            void fetchCompleted(Loader *loader, Document doc, Status status);

        private:
            // TODO
            //Archived articles
            //QValueList<Article> m_archive; // use articles instead
            //void saveArchive(QTextStream &ts);
    };
}

#endif
