/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include "frame.h"

#include <ktabwidget.h>

#include <qptrdict.h>

namespace Akregator
{

class TabWidget:public KTabWidget
{
    Q_OBJECT

    public:
        TabWidget(QWidget * parent = 0, const char *name = 0);
        ~TabWidget();
        void addFrame(Frame *f);
        Frame *currentFrame();
        void removeFrame(Frame *f);
        
        unsigned int tabBarWidthForMaxChars( uint maxLength );
        void setTitle( const QString &title , QWidget* sender);

    public slots:

        void slotSettingsChanged();
    signals:
        
        void currentFrameChanged(Frame *);
        
    private: // methods
        
    private slots:
        
        void slotDetachTab();
        void slotCopyLinkAddress();
        void slotCloseTab();
        void slotCloseRequest(QWidget* widget);
        void contextMenu (int item, const QPoint &p);
        void slotTabChanged(QWidget *w);
        
   private: // attributes

       QPtrDict<Frame> m_frames;
       unsigned int m_CurrentMaxLength;
       QWidget* currentItem;
};

}

#endif
