/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "framemanager.h"
#include "akregatorconfig.h"
#include "frame.h"
#include "actionmanager.h"
#include "browserrun.h"
#include "browserframe.h"
#include "openurlrequest.h"

#include <QAction>
#include <KCharMacroExpander>
#include <kprocess.h>
#include <kshell.h>
#include <kconfiggroup.h>
#include <kparts/event.h>
#include <kparts/guiactivateevent.h>
#include "akregator_debug.h"

#include <QStringList>
#include <QApplication>
#include <QDesktopServices>

namespace Akregator
{

static void setPartGuiActive(KParts::ReadOnlyPart *part, bool active)
{
    if (!part) {
        return;
    }
    // When this event is sent to a KPart, the KPart StatusBarExtension shows or
    // hides its items
    KParts::GUIActivateEvent ev(active);
    QApplication::sendEvent(part, &ev);
}

FrameManager::FrameManager(QWidget *mainWin, QObject *parent) : QObject(parent), m_mainWin(mainWin), m_currentFrame(0)
{
}

FrameManager::~FrameManager()
{
}

void FrameManager::setMainWindow(QWidget *mainWin)
{
    m_mainWin = mainWin;
}

Frame *FrameManager::currentFrame() const
{
    return m_currentFrame;
}

void FrameManager::slotAddFrame(Frame *frame)
{
    m_frames.insert(frame->id(), frame);

    connect(frame, &Frame::signalCanceled, this, &FrameManager::slotSetCanceled);
    connect(frame, &Frame::signalStarted, this, &FrameManager::slotSetStarted);
    connect(frame, &Frame::signalCaptionChanged, this, &FrameManager::slotSetCaption);
    connect(frame, &Frame::signalLoadingProgress, this, &FrameManager::slotSetProgress);
    connect(frame, &Frame::signalCompleted, this, &FrameManager::slotSetCompleted);
    connect(frame, &Frame::signalTitleChanged, this, &FrameManager::slotSetTitle);
    connect(frame, &Frame::signalStatusText, this, &FrameManager::slotSetStatusText
           );

    connect(frame, SIGNAL(signalOpenUrlRequest(Akregator::OpenUrlRequest&)), this, SLOT(slotOpenUrlRequest(Akregator::OpenUrlRequest&)));

    connect(frame, &Frame::signalCanGoBackToggled, this, &FrameManager::slotCanGoBackToggled);
    connect(frame, &Frame::signalCanGoForwardToggled, this, &FrameManager::slotCanGoForwardToggled);
    connect(frame, &Frame::signalIsReloadableToggled, this, &FrameManager::slotIsReloadableToggled
           );

    connect(frame, &Frame::signalIsLoadingToggled, this, &FrameManager::slotIsLoadingToggled);

    setPartGuiActive(frame->part(), false);

    Q_EMIT signalFrameAdded(frame);

    if (m_frames.count() == 1) {
        slotChangeFrame(frame->id());
    }

}

void FrameManager::slotRemoveFrame(int id)
{
    Frame *frame = m_frames.value(id);

    if (!frame) {
        return;
    }
    if (!frame->isRemovable()) {
        return;
    }
    frame->disconnect(this);

    if (m_currentFrame == frame) {
        slotChangeFrame(-1);
    }

    m_frames.insert(id, 0);
    m_frames.remove(id);
    Q_EMIT signalFrameRemoved(id);
    delete frame;
}

Frame *FrameManager::findFrameById(int id) const
{
    return m_frames.value(id);
}

void FrameManager::slotChangeFrame(int frameId)
{
    Frame *frame = m_frames.value(frameId);
    if (frame == m_currentFrame) {
        return;
    }

    Frame *oldFrame = m_currentFrame;
    m_currentFrame = frame;

    if (oldFrame) {
        setPartGuiActive(oldFrame->part(), false);
    }

    if (frame) {
        setPartGuiActive(frame->part(), true);
        slotCanGoBackToggled(frame, frame->canGoBack());
        slotCanGoForwardToggled(frame, frame->canGoForward());
        slotIsReloadableToggled(frame, frame->isReloadable());
        slotIsLoadingToggled(frame, frame->isLoading());

        // TODO: handle removable flag

        switch (frame->state()) {
        case Frame::Started:
            Q_EMIT signalStarted();
            break;
        case Frame::Canceled:
            Q_EMIT signalCanceled(QString());
            break;
        case Frame::Idle:
        case Frame::Completed:
        default:
            Q_EMIT signalCompleted();
        }

        Q_EMIT signalCaptionChanged(frame->caption());
        Q_EMIT signalTitleChanged(frame->title());
        Q_EMIT signalLoadingProgress(frame->progress());
        Q_EMIT signalStatusText(frame->statusText());
    } else {
        Q_EMIT signalCompleted();
        Q_EMIT signalCaptionChanged(QString());
        Q_EMIT signalTitleChanged(QString());
        Q_EMIT signalLoadingProgress(100);
        Q_EMIT signalStatusText(QString());
    }

    Q_EMIT signalCurrentFrameChanged(oldFrame, frame);
}

void FrameManager::slotSetStarted(Frame *frame)
{
    if (frame == m_currentFrame) {
        Q_EMIT signalStarted();
    }
}

void FrameManager::slotSetCanceled(Frame *frame, const QString &reason)
{
    if (frame == m_currentFrame) {
        Q_EMIT signalCanceled(reason);
    }
}

void FrameManager::slotSetCompleted(Frame *frame)
{
    if (frame == m_currentFrame) {
        Q_EMIT signalCompleted();
    }
}

void FrameManager::slotSetProgress(Frame *frame, int progress)
{
    if (frame == m_currentFrame) {
        Q_EMIT signalLoadingProgress(progress);
    }
}

void FrameManager::slotSetIconChanged(Frame *frame, const QIcon &icon)
{
    if (frame != m_currentFrame) {
        return;
    }
    Q_EMIT signalIconChanged(icon);
}

void FrameManager::slotSetCaption(Frame *frame, const QString &caption)
{
    if (frame == m_currentFrame) {
        Q_EMIT signalCaptionChanged(caption);
    }
}

void FrameManager::slotSetTitle(Frame *frame, const QString &title)
{
    if (frame == m_currentFrame) {
        Q_EMIT signalTitleChanged(title);
    }
}

void FrameManager::slotCanGoBackToggled(Frame *frame, bool enabled)
{
    if (frame == m_currentFrame) {
        ActionManager::getInstance()->action("browser_back")->setEnabled(enabled);
    }
}

void FrameManager::slotCanGoForwardToggled(Frame *frame, bool enabled)
{
    if (frame == m_currentFrame) {
        ActionManager::getInstance()->action("browser_forward")->setEnabled(enabled);
    }
}

void FrameManager::slotIsReloadableToggled(Frame *frame, bool enabled)
{
    if (frame == m_currentFrame) {
        ActionManager::getInstance()->action("browser_reload")->setEnabled(enabled);
    }
}

void FrameManager::slotIsLoadingToggled(Frame *frame, bool enabled)
{
    if (frame == m_currentFrame) {
        ActionManager::getInstance()->action("browser_stop")->setEnabled(enabled);
    }
}

void FrameManager::slotSetStatusText(Frame *frame, const QString &statusText)
{
    if (frame == m_currentFrame) {
        Q_EMIT signalStatusText(statusText);
    }
}

void FrameManager::openUrl(OpenUrlRequest &request)
{
    if (request.browserArgs().newTab() || request.browserArgs().forcesNewWindow() || request.options() == OpenUrlRequest::NewTab) {
        int newFrameId = -1;
        Q_EMIT signalRequestNewFrame(newFrameId);
        request.setFrameId(newFrameId);
    }

    if (m_frames.contains(request.frameId())) {
        Frame *frame = m_frames.value(request.frameId());
        if (frame->openUrl(request)) {
            request.setWasHandled(true);
        }
        request.setPart(frame->part());
    }

    if (!request.openInBackground()) {
        Q_EMIT signalSelectFrame(request.frameId());
    }
}

void FrameManager::openInExternalBrowser(const OpenUrlRequest &request)
{
    QUrl url = request.url();
    if (!url.isValid()) {
        return;
    }

    if (!Settings::externalBrowserUseKdeDefault()) {
        QHash<QChar, QString> map;
        map.insert(QLatin1Char('u'), url.url());
        const QString cmd = KMacroExpander::expandMacrosShellQuote(Settings::externalBrowserCustomCommand(), map);
        const QStringList args = KShell::splitArgs(cmd);
        if (!args.isEmpty()) {
            KProcess::startDetached(args);
            return;
        }
    }

    if (request.args().mimeType().isEmpty()) {
        QDesktopServices::openUrl(url);
    } else {
        KRun::runUrl(url, request.args().mimeType(), 0 /*window*/, false, false);
    }
}

void FrameManager::slotOpenUrlRequest(OpenUrlRequest &request, bool useOpenInBackgroundSetting)
{
    qCDebug(AKREGATOR_LOG) << "FrameManager::slotOpenUrlRequest():" << request.debugInfo();

    if (request.options() == OpenUrlRequest::ExternalBrowser) {
        openInExternalBrowser(request);
        return;
    }

    if (useOpenInBackgroundSetting) {
        // Honour user's preference for foreground/background tabs
        if (request.options() == OpenUrlRequest::NewTab ||
                request.browserArgs().newTab()) {
            request.setOpenInBackground(Settings::lMBBehaviour() ==
                                        Settings::EnumLMBBehaviour::OpenInBackground);
        }
    }

    // if no service type is set, determine it using BrowserRun.
    if (request.args().mimeType().isEmpty()) {
        BrowserRun *run = new BrowserRun(request, m_mainWin);
        connect(run, &BrowserRun::signalFoundMimeType, this, &FrameManager::openUrl);
    } else { // serviceType is already set, so we open the page synchronously.
        openUrl(request);
    }

}

void FrameManager::slotBrowserBackAboutToShow()
{
    if (m_currentFrame) {
        m_currentFrame->slotHistoryBackAboutToShow();
    }
}

void FrameManager::slotBrowserForwardAboutToShow()
{
    if (m_currentFrame) {
        m_currentFrame->slotHistoryForwardAboutToShow();
    }
}

void FrameManager::slotBrowserBack()
{
    if (m_currentFrame) {
        m_currentFrame->slotHistoryBack();
    }
}

void FrameManager::slotBrowserForward()
{
    if (m_currentFrame) {
        m_currentFrame->slotHistoryForward();
    }
}

void FrameManager::slotBrowserReload()
{
    if (m_currentFrame) {
        m_currentFrame->slotReload();
    }
}

void FrameManager::slotBrowserStop()
{
    if (m_currentFrame) {
        m_currentFrame->slotStop();
    }
}

void FrameManager::saveProperties(KConfigGroup &config)
{
    //write children
    QStringList strlst;
    QString newPrefix;
    QHash<int, Frame *>::const_iterator i;
    for (i = m_frames.constBegin(); i != m_frames.constEnd(); ++i) {
        // No need to save the main frame
        if (i.value() && qobject_cast<BrowserFrame *>(i.value())) {

            newPrefix = QLatin1Char('T') + QString::number(i.key());
            strlst.append(newPrefix);
            newPrefix.append(QLatin1Char('_'));
            i.value()->saveConfig(config, newPrefix);
        }
    }

    config.writeEntry(QStringLiteral("Children"), strlst);
    config.writeEntry(QStringLiteral("activeChildIndex"),
                      m_frames.key(m_currentFrame));
}

} // namespace Akregator

