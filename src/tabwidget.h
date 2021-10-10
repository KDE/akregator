/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Sashmit Bhaduri <smt@vfemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QTabWidget>

#include <memory>

class QString;

namespace Akregator
{
class Frame;
class OpenUrlRequest;
class TabWidgetPrivate;

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QWidget *parent = nullptr);
    ~TabWidget() override;

public Q_SLOTS:
    void slotWebPageMutedOrAudibleChanged(Akregator::Frame *frame, bool isAudioMuted, bool wasRecentlyAudible);
    void slotCopy();
    void slotZoomChanged(qreal value);

    void slotSetTitle(Akregator::Frame *frame, const QString &title);
    void slotSetIcon(Akregator::Frame *frame, const QIcon &icon);
    void slotSettingsChanged();
    void slotNextTab();
    void slotPreviousTab();
    void slotRemoveCurrentFrame();
    void slotAddFrame(Akregator::Frame *f);
    void slotRemoveFrame(int frameId);
    void slotSelectFrame(int frameId);
    void slotReloadAllTabs();
    void slotActivateTab();
    void slotDetachTab(int index);
    void slotCopyLinkAddress();
    void slotCloseTab();
    void slotPrint();
    void slotPrintPreview();
    void slotFindTextInHtml();
    void slotTextToSpeech();
    void slotSaveLinkAs();
    void slotCopyImageLocation();
    void slotSaveImageOnDisk();
    void slotUnMute();
    void slotMute();
Q_SIGNALS:
    void signalCurrentFrameChanged(int);
    void signalRemoveFrameRequest(int);
    void signalOpenUrlRequest(Akregator::OpenUrlRequest &);
    void signalZoomChangedInFrame(int currentFrame, qreal value);
    void signalPrintInFrame(int currentFrame);
    void signalCopyInFrame(int currentFrame);
    void signalPrintPreviewInFrame(int currentFrame);
    void signalTextToSpeechInFrame(int currentFrame);
    void signalFindTextInFrame(int currentFrame);
    void signalSaveLinkAsInFrame(int currentFrame);
    void signalCopyLinkAsInFrame(int currentFrame);
    void signalCopyImageLocation(int currentFrame);
    void signalSaveImageOnDisk(int currentFrame);
    void signalMute(int currentFrame, bool);

private:
    void tabInserted(int) override;
    void tabRemoved(int) override;

private Q_SLOTS:
    void slotCloseRequest(int index);
    void slotTabChanged(int index);
    void slotCloseAllTab();
    void slotTabContextMenuRequest(const QPoint &pos);
    void slotCloseAllTabExcept(int index);

private:
    void closeAllTabExcept(int index = -1);

private:
    std::unique_ptr<TabWidgetPrivate> const d;
};
} // namespace Akregator

