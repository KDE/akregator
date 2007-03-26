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

#include "actionmanager.h"
#include "akregatorconfig.h"
#include "browserrun.h"
#include "frame.h"
#include "framemanager.h"
#include "openurlrequest.h"

#include <kaction.h>
#include <kprocess.h>
#include <kshell.h>
#include <ktoolinvocation.h>

namespace Akregator {

FrameManager::FrameManager(QWidget* mainWin, QObject* parent) : QObject(parent), m_mainWin(mainWin)
{    
}

FrameManager::~FrameManager()
{
}

void FrameManager::setMainWindow(QWidget* mainWin)
{
    m_mainWin = mainWin;
}

Frame* FrameManager::currentFrame() const
{
    return m_currentFrame;
}

void FrameManager::slotAddFrame(Frame* frame)
{
    m_frames.insert(frame->id(), frame);

    connect(frame, SIGNAL(signalCanceled(Frame*, const QString&)), this, SLOT(slotSetCanceled(Frame*, const QString&)) );
    connect(frame, SIGNAL(signalStarted(Frame*)), this, SLOT(slotSetStarted(Frame*)) );
    connect(frame, SIGNAL(signalCaptionChanged(Frame*, const QString&)), this, SLOT(slotSetCaption(Frame*, const QString&)));
    connect(frame, SIGNAL(signalLoadingProgress(Frame*, int)), this, SLOT(slotSetProgress(Frame*, int)));
    connect(frame, SIGNAL(signalCompleted(Frame*)), this, SLOT(slotSetCompleted(Frame*)));
    connect(frame, SIGNAL(signalTitleChanged(Frame*, const QString&)), this, SLOT(slotSetTitle(Frame*, const QString&)) );
    connect(frame, SIGNAL(signalStatusText(Frame*, const QString&)), this, SLOT(slotSetStatusText(Frame*, const QString&)) );
    
    connect(frame, SIGNAL(signalOpenUrlRequest(OpenUrlRequest&)), this, SLOT(slotOpenUrlRequest(OpenUrlRequest&)) );

    connect(frame, SIGNAL( signalCanGoBackToggled(Frame*, bool)), this, SLOT(slotCanGoBackToggled(Frame*, bool)) );
    connect(frame, SIGNAL( signalCanGoForwardToggled(Frame*, bool)), this, SLOT(slotCanGoForwardToggled(Frame*, bool)) );
    connect(frame, SIGNAL( signalIsReloadableToggled(Frame*, bool)), this, SLOT(slotIsReloadableToggled(Frame*, bool)) );

    connect(frame, SIGNAL( signalIsLoadingToggled(Frame*, bool)), this, SLOT(slotIsLoadingToggled(Frame*, bool)) );
    
    emit signalFrameAdded(frame);

    if (m_frames.count() == 1)
        slotChangeFrame(frame->id());
    
}

void FrameManager::slotRemoveFrame(int id)
{
    Frame* frame = m_frames[id];
    
    if (!frame)
        return;
    
    disconnect(frame, SIGNAL(signalCanceled(Frame*, const QString&)), this, SLOT(slotSetCanceled(Frame*, const QString&)) );
    disconnect(frame, SIGNAL(signalStarted(Frame*)), this, SLOT(slotSetStarted(Frame*)) );
    disconnect(frame, SIGNAL(signalCaptionChanged(Frame*, const QString&)), this, SLOT(slotSetCaption(Frame*, const QString&)));
    disconnect(frame, SIGNAL(signalLoadingProgress(Frame*, int)), this, SLOT(slotSetProgress(Frame*, int)));
    disconnect(frame, SIGNAL(signalCompleted(Frame*)), this, SLOT(slotSetCompleted(Frame*)));
    disconnect(frame, SIGNAL(signalTitleChanged(Frame*, const QString&)), this, SLOT(slotSetTitle(Frame*, const QString&)) );
    disconnect(frame, SIGNAL(signalStatusText(Frame*, const QString&)), this, SLOT(slotSetStatusText(Frame*, const QString&)) );

    if (m_currentFrame == frame)
    {
        slotChangeFrame(-1);
    }
    
    m_frames.remove(frame->id());
    emit signalFrameRemoved(frame->id());
}

Frame* FrameManager::findFrameById(int id) const
{
    return m_frames[id];
}

void FrameManager::slotChangeFrame(int frameId)
{
    Frame* frame = m_frames[frameId];
    if (frame == m_currentFrame)
        return;
    
    Frame* oldFrame = m_currentFrame;
    m_currentFrame = frame;
    
    if (frame)
    {
        slotCanGoBackToggled(frame, frame->canGoBack());
        slotCanGoForwardToggled(frame, frame->canGoForward());
        slotIsReloadableToggled(frame, frame->isReloadable());
        slotIsLoadingToggled(frame, frame->isLoading());
        
        // TODO: handle removable flag
        
        switch (frame->state())
        {
            case Frame::Started:
                emit signalStarted();
                break;
            case Frame::Canceled:
                emit signalCanceled(QString::null);
                break;
            case Frame::Idle:
            case Frame::Completed:
            default:
                emit signalCompleted();
        }

        emit signalCaptionChanged(frame->caption());
        emit signalTitleChanged(frame->title());
        emit signalLoadingProgress( frame->progress());
        emit signalStatusText( frame->statusText());
    }
    else
    {
        emit signalCompleted();
        emit signalCaptionChanged(QString::null);
        emit signalTitleChanged(QString::null);
        emit signalLoadingProgress(100);
        emit signalStatusText(QString::null);
    }

    emit signalCurrentFrameChanged(oldFrame, frame);
}

void FrameManager::slotSetStarted(Frame* frame)
{
    if (frame == m_currentFrame)
        emit signalStarted();
}

void FrameManager::slotSetCanceled(Frame* frame, const QString& reason)
{
    if (frame == m_currentFrame)
        emit signalCanceled(reason);
}

void FrameManager::slotSetCompleted(Frame* frame)
{
    if (frame == m_currentFrame)
        emit signalCompleted();
}

void FrameManager::slotSetProgress(Frame* frame, int progress)
{
    if (frame == m_currentFrame)
        emit signalLoadingProgress(progress);
}

void FrameManager::slotSetCaption(Frame* frame, const QString& caption)
{
    if (frame == m_currentFrame)
        emit signalCaptionChanged(caption);
}

void FrameManager::slotSetTitle(Frame* frame, const QString& title)
{
    if (frame == m_currentFrame)
        emit signalTitleChanged(title);
}

void FrameManager::slotCanGoBackToggled(Frame* frame, bool enabled)
{
    if (frame == m_currentFrame)
        ActionManager::getInstance()->action("browser_back")->setEnabled(enabled);
}

void FrameManager::slotCanGoForwardToggled(Frame* frame, bool enabled)
{
    if (frame == m_currentFrame)
        ActionManager::getInstance()->action("browser_forward")->setEnabled(enabled);
}

void FrameManager::slotIsReloadableToggled(Frame* frame, bool enabled)
{
    if (frame == m_currentFrame)
        ActionManager::getInstance()->action("browser_reload")->setEnabled(enabled);
}

void FrameManager::slotIsLoadingToggled(Frame* frame, bool enabled)
{
    if (frame == m_currentFrame)
        ActionManager::getInstance()->action("browser_stop")->setEnabled(enabled);
}

void FrameManager::slotSetStatusText(Frame* frame, const QString& statusText)
{
    if (frame == m_currentFrame)
        emit signalStatusText(statusText);
}

void FrameManager::openUrl(OpenUrlRequest& request)
{
    if (request.args().newTab() || request.args().forcesNewWindow() || request.options() == OpenUrlRequest::NewTab)
    {
        int newFrameId = -1;
        emit signalRequestNewFrame(newFrameId);
        request.setFrameId(newFrameId);
    }
    
    if (m_frames.contains(request.frameId()))
    {
        Frame* frame = m_frames.value(request.frameId());
        frame->openUrl(request);
        if (frame->part())
            request.setPart(frame->part());
    }
    
    if (!request.openInBackground())
        emit signalSelectFrame(request.frameId());
}
void FrameManager::openInExternalBrowser(const OpenUrlRequest& request)
{
    KUrl url = request.url(); 
    if (!url.isValid())
        return;
   
    if (Settings::externalBrowserUseKdeDefault())
    {
        if (request.args().serviceType.isEmpty()) 
            KToolInvocation::self()->invokeBrowser(url.url(), "0");
        else
            KRun::runUrl(url, request.args().serviceType, false, false);
    }
    else
    {
        QString cmd = Settings::externalBrowserCustomCommand();
        QString urlStr = url.url();
        cmd.replace(QRegExp("%u"), urlStr);
        KProcess *proc = new KProcess;
        QStringList cmdAndArgs = KShell::splitArgs(cmd);
        *proc << cmdAndArgs;
        proc->start(KProcess::DontCare);
        delete proc;
    }
}
void FrameManager::slotFoundMimeType(const OpenUrlRequest& request)
{
    OpenUrlRequest req2 = request;
    openUrl(req2);
}

void FrameManager::slotOpenUrlRequest(OpenUrlRequest& request)
{
    kDebug() << "FrameManager::slotOpenUrlRequest(): " << request.debugInfo() << endl;
    
    if (request.options() == OpenUrlRequest::ExternalBrowser)
    {
        openInExternalBrowser(request);
        return;
    }
    // if no service type is set, determine it using BrowserRun.
    if (request.args().serviceType.isEmpty())
    {
        BrowserRun* run = new BrowserRun(request, m_mainWin);
        connect(run, SIGNAL(signalFoundMimeType(const OpenUrlRequest&)), this, SLOT(slotFoundMimeType(const OpenUrlRequest&)));
    }
    else // serviceType is already set, so we open the page synchronously.
    {
        openUrl(request);
    }
    
}

void FrameManager::slotBrowserBackAboutToShow()
{
    if (m_currentFrame)
        m_currentFrame->slotHistoryBackAboutToShow();
}

void FrameManager::slotBrowserForwardAboutToShow()
{
    if (m_currentFrame)
        m_currentFrame->slotHistoryForwardAboutToShow();
}

void FrameManager::slotBrowserBack()
{
    if (m_currentFrame)
        m_currentFrame->slotHistoryBack();
}

void FrameManager::slotBrowserForward()
{
    if (m_currentFrame)
        m_currentFrame->slotHistoryForward();
}

void FrameManager::slotBrowserReload()
{
    if (m_currentFrame)
        m_currentFrame->slotReload();
}

void FrameManager::slotBrowserStop()
{
    if (m_currentFrame)
        m_currentFrame->slotStop();
}

} // namespace Akregator 

#include "framemanager.moc"
