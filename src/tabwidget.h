/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <qptrlist.h>
#include <ktabwidget.h>

class QString;

namespace Akregator
{

class Frame;

class TabWidget : public KTabWidget
{
    Q_OBJECT

    public:
        TabWidget(QWidget * parent = 0, const char *name = 0);
        virtual ~TabWidget();

        void addFrame(Frame *f);
        Frame* currentFrame();
        void removeFrame(Frame *f);
        QPtrList<Frame> frames() const;

    public slots:

        void slotSetTitle(Frame* frame, const QString& title);
        void slotSettingsChanged();
        void slotNextTab();
        void slotPreviousTab();
        void slotRemoveCurrentFrame();

    signals:
        
        void currentFrameChanged(Frame *);
        
    protected slots:

        virtual void initiateDrag(int tab);
        
    private: // methods
        
        uint tabBarWidthForMaxChars( uint maxLength );
        void setTitle( const QString &title , QWidget* sender);


    private slots:
        
        void slotDetachTab();
        void slotCopyLinkAddress();
        void slotCloseTab();
        void slotCloseRequest(QWidget* widget);
        void contextMenu (int item, const QPoint &p);
        void slotTabChanged(QWidget *w);
        
   private:

       class TabWidgetPrivate;
       TabWidgetPrivate* d;
};

}

#endif
