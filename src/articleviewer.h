/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef ARTICLEVIEWER_H
#define ARTICLEVIEWER_H

#include <klocale.h>

#include <qcolor.h>
#include <qfont.h>
#include <qpaintdevicemetrics.h>

#include "viewer.h"
#include "myarticle.h"

namespace Akregator
{
    class Feed;

    class ArticleViewer : public Viewer
    {
        Q_OBJECT
        public:
            ArticleViewer(QWidget* parent, const char* name);
            void openDefault();
            void show(Feed *f, bool headers=true);
            void show(Feed *f, MyArticle);
            void generateCSS();
            void reload();
            void beginWriting();
            void endWriting();
            void slotOpenLinkInternal();
                

        private:
            QString formatArticle(Feed *f, MyArticle a);
            /**
             * Display article in external browser.
             */
            void displayInExternalBrowser(const KURL& url);
            
            QString m_htmlHead;
            QPaintDeviceMetrics m_metrics;
            QString m_currentText;

        private slots:
            bool slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
    };
}

#endif // ARTICLEVIEWER_H
