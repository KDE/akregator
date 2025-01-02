/*
   SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregatorpart_export.h"
#include "frame.h"

class KActionCollection;

namespace Akregator
{
class ArticleViewerWebEngineWidgetNg;
class AKREGATOR_EXPORT WebEngineFrame : public Frame
{
    Q_OBJECT
public:
    explicit WebEngineFrame(KActionCollection *ac, QWidget *parent = nullptr);
    ~WebEngineFrame() override;

    [[nodiscard]] QUrl url() const override;
    [[nodiscard]] bool openUrl(const OpenUrlRequest &request) override;
    void loadConfig(const KConfigGroup &, const QString &) override;
    [[nodiscard]] bool saveConfig(KConfigGroup &, const QString &) override;

    [[nodiscard]] qreal zoomFactor() const override;

public Q_SLOTS:
    void slotReload() override;
    void slotStop() override;

    void slotZoomChangeInFrame(int frameId, qreal value);
    void slotCopyInFrame(int frameId);
    void slotPrintInFrame(int frameId);
    void slotPrintPreviewInFrame(int frameId);
    void slotFindTextInFrame(int frameId);
    void slotTextToSpeechInFrame(int frameId);
    void slotSaveLinkAsInFrame(int frameId);
    void slotCopyLinkAsInFrame(int frameId);
    void slotSaveImageOnDiskInFrame(int frameId);
    void slotCopyImageLocationInFrame(int frameId);
    void slotMute(int frameId, bool mute);

Q_SIGNALS:
    void signalIconChanged(Akregator::Frame *, const QIcon &icon);
    void webPageMutedOrAudibleChanged(Akregator::Frame *, bool isAudioMuted, bool wasRecentlyAudible);

private Q_SLOTS:
    void slotTitleChanged(const QString &title);
    void slotProgressChanged(int progress);
    void slotLoadStarted();
    void slotLoadFinished();
    void slotWebPageMutedOrAudibleChanged(bool isAudioMuted, bool wasRecentlyAudible);

private:
    void loadUrl(const QUrl &url);
    Akregator::ArticleViewerWebEngineWidgetNg *mArticleViewerWidgetNg = nullptr;
};
}
