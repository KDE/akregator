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

#ifndef AKREGATOR_FRAMEMANAGER_H
#define AKREGATOR_FRAMEMANAGER_H

#include <QObject>
#include <QSet>

#include "frame.h"

class QString;

class KUrl;

namespace KParts
{
    class URLArgs;
}

namespace Akregator {

class FrameManager : public QObject
{
    Q_OBJECT

    public:

        FrameManager(QObject* parent=0);
        virtual ~FrameManager();

        Frame* currentFrame() const;

        void addFrame(Frame* frame);
        void removeFrame(Frame* frame);
       
    public slots:

        void slotChangeFrame(Frame* frame);
        void slotOpenURLRequest(Frame*, const KURL&, const KParts::URLArgs& args, Frame::OpenURLOptions options);

        void slotBrowserBack();
        void slotBrowserForward();
        void slotBrowserReload();
        void slotBrowserStop();

    signals:

        void signalFrameAdded(Frame*);
        void signalFrameRemoved(Frame*);

        // TODO: merge signals
        /** emitted when the active frame is switched */
        void signalCurrentFrameChanged(Frame*);

        /** 
         * emitted when the active frame is switched
         * @param the the deactivated frame
         * @param  the activated frame
         */
        void signalCurrentFrameChanged(Frame*, Frame*);

        void signalStarted();
        void signalCanceled(const QString&);
        void signalCompleted();
        void signalCaptionChanged(const QString&);
        void signalTitleChanged(const QString&);
        void signalLoadingProgress(int);
        void signalStatusText(const QString&);

    protected slots:

        void slotSetStarted(Frame* frame);
        void slotSetCanceled(Frame* frame, const QString& reason);
        void slotSetCompleted(Frame* frame);
        void slotSetProgress(Frame* frame, int progress);
        void slotSetCaption(Frame* frame, const QString& caption);
        void slotSetTitle(Frame* frame, const QString& title);
        void slotSetStatusText(Frame* frame, const QString& statusText);

        void slotCanGoBackToggled(Frame*, bool);
        void slotCanGoForwardToggled(Frame*, bool);
        void slotIsReloadableToggled(Frame*, bool);
        void slotIsLoadingToggled(Frame*, bool);

        void slotFoundMimeType(Frame* frame, const KURL& url, const KParts::URLArgs& args, const QString& mimetype);

    private:

        Frame* m_currentFrame;
        QSet<Frame*> m_frames;
};

} // namespace Akregator

#endif // AKREGATOR_FRAMEMANAGER_H
