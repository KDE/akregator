/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef _AKREGATORPART_H_
#define _AKREGATORPART_H_

#include <kparts/browserextension.h>
#include <kparts/part.h>
#include <kurl.h>

#include "akregator_partiface.h"

class KParts::BrowserExtension;
class KAboutData;
class KConfig;
class KRecentFilesAction;

namespace Akregator
{
    class aKregatorView;
    class aKregatorPart;
    class Feed;
    class MyArticle;

    class BrowserExtension : public KParts::BrowserExtension
    {
        Q_OBJECT

        public:
            BrowserExtension( aKregatorPart *p, const char *name );
        public slots:
            void saveSettings();
        private:
            aKregatorPart *m_part;
    };
    
    /**
     * This is a RSS Aggregator "Part". It does all the real work.
     * It is also embeddable into other applications (e.g. for use in Kontact).
     */
    class aKregatorPart : public KParts::ReadWritePart, virtual public aKregatorPartIface
    {
        Q_OBJECT
        public:
           typedef KParts::ReadWritePart inherited;

            /**
             * Default constructor.
             */
            aKregatorPart(QWidget *parentWidget, const char *widgetName,
                          QObject *parent, const char *name, const QStringList&);

	    /**
             * Destructor.
             */
            virtual ~aKregatorPart();

            /**
             * This is a virtual function inherited from KParts::ReadWritePart.
             * A shell will use this to inform this Part if it should act
             * read-only
             */
            virtual void setReadWrite(bool rw);

            /**
             * Reimplemented to disable and enable Save action
             */
            virtual void setModified(bool modified);


            /**
             * Create KAboutData for this KPart.
             */
            static KAboutData *createAboutData();

            void changePart(KParts::ReadOnlyPart *p);

            void setStatusBar(const QString &text);
            void setProgress(int percent);
            void setStarted();
            void setCompleted();
            void setCanceled(const QString &s);
            
            void setTotalUnread(int unread);
            void newArticle(Feed *src, const MyArticle &a);

            void setCaption(const QString &text);
            
            virtual bool openURL(const KURL& url);
            virtual bool closeURL();
            virtual void openStandardFeedList();

            bool loading (){return m_loading;}
            
            virtual void fetchFeedUrl(const QString&);
            virtual void fetchAllFeeds();
            
            /**
             * Add a feed to a group.
             * @param url The URL of the feed to add.
             * @param group The name of the folder into which the feed is added.
             *
             * If the group does not exist, it is created.  The feed is added as the last member
             * of the group.
             */
            virtual void addFeedToGroup(const QString& url, const QString& group);
            
            /** session management **/
            virtual void readProperties(KConfig* config);
            virtual void saveProperties(KConfig* config);
            
        protected:
            /**
             * This must be implemented by each part
             */
            virtual bool openFile();

            /**
             * This must be implemented by each read-write part
             */
            virtual bool saveFile();
            
            void importFile(QString fileName);
            void exportFile(QString fileName);

        signals:
            void partChanged(KParts::ReadOnlyPart *p);
            
        public slots:
            virtual void saveSettings();

        protected slots:
            void fileOpen();
            bool fileSaveAs();
            void fileImport();
            void fileExport();
            void openURLDelayed();

        private:
            void readRecentFileEntries();
            bool isStandardFeedList();
            bool populateStandardFeeds();

            bool m_loading;

            KURL m_delayURL;            
            int m_totalUnread;
            KParts::BrowserExtension *m_extension;
            KRecentFilesAction *recentFilesAction;
            static KAboutData* s_about;
            aKregatorView* m_view;
    };
}

#endif // _AKREGATORPART_H_

// vim: set et ts=4 sts=4 sw=4:
