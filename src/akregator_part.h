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
#include <kaboutdata.h>
#include <kaction.h>

#include "akregator_partiface.h"

class KParts::BrowserExtension;

namespace Akregator
{
    class aKregatorView;
    class aKregatorPart;

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
            void setTotalUnread(int unread);
            void setCaption(const QString &text);
            
            void startOperation();
            void endOperation();
            void operationError(const QString &msg);

            virtual bool openURL(const KURL& url);
            virtual bool closeURL();
                
            virtual void fetchFeedUrl(const QString&);
            
            virtual void addFeedToGroup(const QString& url, const QString& group);
            
            
        protected:
            /**
             * This must be implemented by each part
             */
            virtual bool openFile();

            /**
             * This must be implemented by each read-write part
             */
            virtual bool saveFile();

            void importFile(QString file_name);

        signals:
            void partChanged(KParts::ReadOnlyPart *p);
            
        public slots:
            virtual void saveSettings();

        protected slots:
            void fileOpen();
            bool fileSaveAs();
            void fileImport();

        private:
            void readRecentFileEntries();

            bool m_loading;
            
            int m_totalUnread;
            KParts::BrowserExtension *m_extension;
            KRecentFilesAction *recentFilesAction;
            static KAboutData* s_about;
            aKregatorView* m_view;
    };
}

#endif // _AKREGATORPART_H_

// vim: set et ts=4 sts=4 sw=4:
