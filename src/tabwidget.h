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

#ifndef AKREGATOR_TABWIDGET_H
#define AKREGATOR_TABWIDGET_H

#include <ktabwidget.h>

class QString;

namespace Akregator
{

class Frame;
class OpenUrlRequest;

class TabWidget : public KTabWidget
{
    Q_OBJECT

    public:
        explicit TabWidget(QWidget * parent = 0);
        ~TabWidget();

    public slots:

        void slotSetTitle(Akregator::Frame* frame, const QString& title);
        void slotSetIcon(Akregator::Frame* frame, const QIcon& icon);
        void slotSettingsChanged();
        void slotNextTab();
        void slotPreviousTab();
        void slotRemoveCurrentFrame();
        void slotAddFrame(Akregator::Frame* f);
        void slotRemoveFrame(int frameId);
        void slotSelectFrame(int frameId);

    Q_SIGNALS:
        void signalCurrentFrameChanged(int);
        void signalRemoveFrameRequest(int);
        void signalOpenUrlRequest(Akregator::OpenUrlRequest&);

    private:
        /*reimpl*/ void tabInserted( int );
        /*reimpl*/ void tabRemoved( int );

    private Q_SLOTS:
        void initiateDrag(int tab);
        void slotDetachTab();
        void slotCopyLinkAddress();
        void slotCloseTab();
        void slotCloseRequest(QWidget* widget);
        void contextMenu (int item, const QPoint &p);
        void slotTabChanged(QWidget *w);

   private:

       class Private;
       Private* const d;
};

} // namespace Akregator

#endif // AKREGATOR_TABWIDGET_H
