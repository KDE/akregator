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

#include <QHash>
#include <QObject>

class QString;



namespace Akregator {

class Frame;
class OpenUrlRequest;
    
class FrameManager : public QObject
{
    Q_OBJECT

    public:

        explicit FrameManager(QWidget* mainWin=0, QObject* parent=0);
        ~FrameManager();

        Frame* currentFrame() const;
        
        Frame* findFrameById(int id) const;
        
        void setMainWindow(QWidget* mainWin);

    public slots:

        void slotAddFrame(Frame* frame);
        void slotRemoveFrame(int frameId);
               
        void slotChangeFrame(int frameId);
        void slotOpenUrlRequest(Akregator::OpenUrlRequest& request);

        void slotBrowserBack();
        void slotBrowserForward();
        void slotBrowserReload();
        void slotBrowserStop();
        void slotBrowserBackAboutToShow();
        void slotBrowserForwardAboutToShow();
        
    signals:

        void signalFrameAdded(Akregator::Frame*);
        void signalFrameRemoved(int id);

        void signalRequestNewFrame(int& id);
        
        void signalSelectFrame(int id);
        /** 
         * emitted when the active frame is switched
         * @param deactivated the the deactivated frame
         * @param activated the activated frame
         */
        void signalCurrentFrameChanged(Akregator::Frame* deactivated, Akregator::Frame* activated);

        void signalStarted();
        void signalCanceled(const QString&);
        void signalCompleted();
        void signalCaptionChanged(const QString&);
        void signalTitleChanged(const QString&);
        void signalLoadingProgress(int);
        void signalStatusText(const QString&);

    private:
        
        void openUrl(OpenUrlRequest& request);
        
        void openInExternalBrowser(const OpenUrlRequest& request);
        
    private slots:

        void slotSetStarted(Akregator::Frame* frame);
        void slotSetCanceled(Akregator::Frame* frame, const QString& reason);
        void slotSetCompleted(Akregator::Frame* frame);
        void slotSetProgress(Akregator::Frame* frame, int progress);
        void slotSetCaption(Akregator::Frame* frame, const QString& caption);
        void slotSetTitle(Akregator::Frame* frame, const QString& title);
        void slotSetStatusText(Akregator::Frame* frame, const QString& statusText);

        void slotCanGoBackToggled(Akregator::Frame*, bool);
        void slotCanGoForwardToggled(Akregator::Frame*, bool);
        void slotIsReloadableToggled(Akregator::Frame*, bool);
        void slotIsLoadingToggled(Akregator::Frame*, bool);

        void slotFoundMimeType(const Akregator::OpenUrlRequest& request);

    private:

        QWidget* m_mainWin;
        Frame* m_currentFrame;
        QHash<int, Frame*> m_frames;
};

} // namespace Akregator

#endif // AKREGATOR_FRAMEMANAGER_H
