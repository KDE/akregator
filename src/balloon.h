/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri (smt@vfemail.net                *
 *                                                                         *
 *	 based on kopeteballoon.cpp:                                           *
 *    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>        *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef BALLOON_H
#define BALLOON_H

#include <qwidget.h>

class QListView;

namespace Akregator {

class Balloon : public QWidget
{
    Q_OBJECT

    public:
        Balloon(const QString &text);

        void setAnchor(const QPoint &anchor);
        void updateMask();
        void addArticle(const QString&feed, const QPixmap&p, const QString&art);

    signals:
        void signalButtonClicked();
        void signalIgnoreButtonClicked();
        void signalBalloonClicked();

    private:
        QPoint mAnchor;
        QListView *m_articleList;
};

} // namespace Akregator
#endif

// vim: set noet ts=4 sts=4 sw=4:

