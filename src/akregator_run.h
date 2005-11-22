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

#ifndef AKREGATOR_RUN_H 
#define AKREGATOR_RUN_H

#include <kparts/browserrun.h>

namespace Akregator
{

class Viewer;

class BrowserRun : public KParts::BrowserRun
{
    Q_OBJECT
    public:
        /** indicates how HTML pages should be opened. It is passed in the constructor and sent back via the openInViewer signal. This is a workaround to fix opening of non-HTML mimetypes in 3.5, which will be refactored for KDE4 anyway. For 3.5.x it's the easiest way to fix the problem without changing too much code TODO KDE4: refactor, remove this enum  */
        enum OpeningMode 
        {
            CURRENT_TAB,
            NEW_TAB_FOREGROUND,
            NEW_TAB_BACKGROUND,
            EXTERNAL
        };

        BrowserRun(QWidget* mainWindow, Viewer* currentViewer, const KURL& url, const KParts::URLArgs& args, OpeningMode mode);
        virtual ~BrowserRun();

    signals:

        void signalOpenInViewer(const KURL&, Akregator::Viewer*, Akregator::BrowserRun::OpeningMode);

    protected:
	    virtual void foundMimeType(const QString& type);

    private slots:
        void slotViewerDeleted();

    private:
        OpeningMode m_openingMode;
        Viewer* m_currentViewer;
};

}

#endif

// vim: set et ts=4 sts=4 sw=4:
