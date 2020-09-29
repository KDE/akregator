/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WEBENGINEFRAME_H
#define WEBENGINEFRAME_H

#include "frame.h"
#include "akregatorpart_export.h"

class KActionCollection;

namespace Akregator {
class ArticleViewerWebEngineWidgetNg;
class AKREGATORPART_EXPORT WebEngineFrame : public Frame
{
    Q_OBJECT
public:
    explicit WebEngineFrame(KActionCollection *ac, QWidget *parent = nullptr);
    ~WebEngineFrame() override;

    Q_REQUIRED_RESULT QUrl url() const override;
    Q_REQUIRED_RESULT bool openUrl(const OpenUrlRequest &request) override;
    void loadConfig(const KConfigGroup &, const QString &) override;
    Q_REQUIRED_RESULT bool saveConfig(KConfigGroup &, const QString &) override;

    Q_REQUIRED_RESULT qreal zoomFactor() const override;

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

#endif
