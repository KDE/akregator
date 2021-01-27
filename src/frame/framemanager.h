/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_FRAMEMANAGER_H
#define AKREGATOR_FRAMEMANAGER_H

#include <QHash>
#include <QObject>
#include <QPointer>

#include "akregator_export.h"

class QIcon;
class QString;
class KConfigGroup;

namespace Akregator
{
class Frame;
class OpenUrlRequest;

class AKREGATOR_EXPORT FrameManager : public QObject
{
    Q_OBJECT

public:
    explicit FrameManager(QObject *parent = nullptr);
    ~FrameManager();

    Frame *currentFrame() const;

    Frame *findFrameById(int id) const;

    /** session management **/
    void saveProperties(KConfigGroup &config);

public Q_SLOTS:

    void slotAddFrame(Frame *frame);
    void slotRemoveFrame(int frameId);

    void slotChangeFrame(int frameId);
    void slotOpenUrlRequest(Akregator::OpenUrlRequest &request, bool useOpenInBackgroundSetting = true);

Q_SIGNALS:

    void signalFrameAdded(Akregator::Frame *);
    void signalFrameRemoved(int id);

    void signalRequestNewFrame(int &id);

    void signalSelectFrame(int id);
    /**
     * emitted when the active frame is switched
     * @param deactivated the deactivated frame
     * @param activated the activated frame
     */
    void signalCurrentFrameChanged(Akregator::Frame *deactivated, Akregator::Frame *activated);

    void signalStarted();
    void signalCanceled(const QString &);
    void signalCompleted();
    void signalCaptionChanged(const QString &);
    void signalTitleChanged(const QString &);
    void signalIconChanged(const QIcon &);
    void signalLoadingProgress(int);
    void signalStatusText(const QString &);

private:
    void openInExternalBrowser(const OpenUrlRequest &request);

private Q_SLOTS:
    void openUrl(Akregator::OpenUrlRequest &request);
    void slotSetStarted(Akregator::Frame *frame);
    void slotSetCanceled(Akregator::Frame *frame, const QString &reason);
    void slotSetCompleted(Akregator::Frame *frame);
    void slotSetProgress(Akregator::Frame *frame, int progress);
    void slotSetCaption(Akregator::Frame *frame, const QString &caption);
    void slotSetTitle(Akregator::Frame *frame, const QString &title);
    void slotSetStatusText(Akregator::Frame *frame, const QString &statusText);

private:
    QPointer<Frame> m_currentFrame;
    QHash<int, Frame *> m_frames;
};
} // namespace Akregator

#endif // AKREGATOR_FRAMEMANAGER_H
