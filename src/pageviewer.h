/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef PAGEVIEWER_H
#define PAGEVIEWER_H

#include <khtml_part.h>

namespace Akregator
{
    class PageViewer : public KHTMLPart
    {
        Q_OBJECT
        public:
            PageViewer(QWidget* parent, const char* name);

        signals:
           void urlClicked(const KURL& url);

        private slots:
           void slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args);
    };
}

#endif // PAGEVIEWER_H
