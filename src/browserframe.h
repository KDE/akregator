/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#ifndef AKREGATOR_BROWSERFRAME_H
#define AKREGATOR_BROWSERFRAME_H

#include "frame.h"

class QString;
class QWidget;

class KUrl;

namespace KParts 
{
    class ReadOnlyPart;
}

namespace Akregator {


class BrowserFrame : public Frame
{
    Q_OBJECT

    public:

        BrowserFrame(QWidget* parent=0);
        virtual ~BrowserFrame();

        virtual KUrl url() const;

        virtual KParts::ReadOnlyPart* part() const;

        virtual bool canGoForward() const;
        virtual bool canGoBack() const;
        virtual bool isReloadable() const;
        virtual bool isLoading() const;

        virtual bool openURL(const KUrl& url, const QString& mimetype="text/html");

    public slots:

        virtual void slotHistoryForward();
        virtual void slotHistoryBack();
        virtual void slotReload();
        virtual void slotStop();

        virtual void slotPaletteOrFontChanged();

    protected slots:

        void slotOpenURLRequestDelayed(const KUrl&, const KParts::URLArgs&);
        void slotCreateNewWindow(const KUrl&, const KParts::URLArgs&);
        void slotOpenURLNotify();
        void slotSetLocationBarURL(const QString& url);
        void slotSetIconURL(const KUrl& url);
        void slotSpeedProgress(int);

    private:
        class HistoryEntry;
        class BrowserFramePrivate;
        BrowserFramePrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_BROWSERFRAME_H
