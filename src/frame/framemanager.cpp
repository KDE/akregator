/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "framemanager.h"
#include "akregatorconfig.h"
#include "frame.h"
#include "actionmanager.h"
#include "openurlrequest.h"
#include <KMacroExpander>
#include <KProcess>
#include <KShell>
#include <KConfigGroup>
#include "akregator_debug.h"
#include <webengine/webengineframe.h>
#include <QStringList>
#include <QDesktopServices>
#include <KIO/JobUiDelegate>
#include <KIO/OpenUrlJob>

using namespace Akregator;

FrameManager::FrameManager(QObject *parent)
    : QObject(parent)
    , m_currentFrame(nullptr)
{
}

FrameManager::~FrameManager()
{
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
    connect(frame, &Frame::signalStatusText, this, &FrameManager::slotSetStatusText);

    connect(frame, &Frame::signalOpenUrlRequest, this, [this](OpenUrlRequest &request) {
        slotOpenUrlRequest(request);
    });

    //setPartGuiActive(frame->part(), false);

    Q_EMIT signalFrameAdded(frame);

    if (m_frames.count() == 1) {
        slotChangeFrame(frame->id());
    }
}

void FrameManager::slotRemoveFrame(int id)
{
    Frame *frame = m_frames.value(id);

    if (!frame || !frame->isRemovable()) {
        return;
    }
    frame->disconnect(this);

    if (m_currentFrame == frame) {
        slotChangeFrame(-1);
    }

    m_frames.insert(id, nullptr);
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

    //if (oldFrame) {
    //    setPartGuiActive(oldFrame->part(), false);
    //}

    if (frame) {
        //setPartGuiActive(frame->part(), true);

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

void FrameManager::slotSetStatusText(Frame *frame, const QString &statusText)
{
    if (frame == m_currentFrame) {
        Q_EMIT signalStatusText(statusText);
    }
}

void FrameManager::openUrl(OpenUrlRequest &request)
{
    if (request.browserArgs().newTab() || request.browserArgs().forcesNewWindow() || request.options() == OpenUrlRequest::NewTab || (m_currentFrame->id() == 0)) {
        int newFrameId = -1;
        Q_EMIT signalRequestNewFrame(newFrameId);
        request.setFrameId(newFrameId);
    } else {
        request.setFrameId(m_currentFrame->id());
    }
    if (m_frames.contains(request.frameId())) {
        Frame *frame = m_frames.value(request.frameId());
        if (frame->openUrl(request)) {
            request.setWasHandled(true);
        }
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
        KIO::OpenUrlJob *job = new KIO::OpenUrlJob(url, request.args().mimeType());
        job->start();
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
        if (request.options() == OpenUrlRequest::NewTab
            || request.browserArgs().newTab()) {
            request.setOpenInBackground(Settings::lMBBehaviour()
                                        == Settings::EnumLMBBehaviour::OpenInBackground);
        }
    }
    openUrl(request);
}

void FrameManager::saveProperties(KConfigGroup &config)
{
    //write children
    QStringList strlst;
    QHash<int, Frame *>::const_iterator i;
    QHash<int, Frame *>::const_iterator end(m_frames.constEnd());
    for (i = m_frames.constBegin(); i != end; ++i) {
        // No need to save the main frame
        Frame *currentFrame = i.value();
        if (currentFrame && qobject_cast<WebEngineFrame *>(currentFrame)) {
            const QString newPrefix = QLatin1Char('T') + QString::number(i.key());
            if (currentFrame->saveConfig(config, newPrefix + QLatin1Char('_'))) {
                strlst.append(newPrefix);
                if (currentFrame == m_currentFrame) {
                    config.writeEntry(QStringLiteral("CurrentTab"), newPrefix);
                }
            }
        }
    }
    config.writeEntry(QStringLiteral("Children"), strlst);
}
