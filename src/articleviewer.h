/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef ARTICLEVIEWER_H
#define ARTICLEVIEWER_H

#include <khtml_part.h>
#include <qcolor.h>
#include <qfont.h>
#include <qpaintdevicemetrics.h>
#include "myarticle.h"

namespace Akregator
{

    class Feed;

    class ArticleViewer : public KHTMLPart
    {
        Q_OBJECT
        public:
            ArticleViewer(QWidget* parent, const char* name);
            void openDefault();
            void show(Feed *f, MyArticle);
	    void generateCSS();
	    void reload();	
        private:
	    QString m_htmlHead;
            QPaintDeviceMetrics m_metrics;
            QString m_currentText;
    };
}

#endif // ARTICLEVIEWER_H
