/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <ktabwidget.h>

#ifdef HAVE_FRAME
#include <qptrdict.h>
#include "frame.h"
#else
#include <qdict.h>
#endif

namespace Akregator
{

    class TabWidget:public KTabWidget
    {
        Q_OBJECT public:
            TabWidget(QWidget * parent = 0, const char *name = 0);
            ~TabWidget();
#ifdef HAVE_FRAME
//            void addFrame(Frame *f);
//            Frame *currentFrame();
//            void removeFrame(Frame *f);
#endif


            unsigned int tabBarWidthForMaxChars( uint maxLength );
            void setTitle( const QString &title , QWidget* sender);

#ifdef HAVE_FRAME            
//            signals:
//                void currentFrameChanged(Frame *);
#endif
            private:
#ifdef HAVE_FRAME
//                QPtrDict <Frame>m_frames;
#else
                QDict <QWidget>m_titleDict;
#endif
                unsigned int m_CurrentMaxLength;
                QWidget *currentItem;
            private slots:
                void slotDetachTab();
                void slotCloseTab();
                void contextMenu (int item, const QPoint &p);
#ifdef HAVE_FRAME
//                void slotTabChanged(QWidget *w);
#endif
   };
}

#endif
