/*
   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "frame.h"

namespace Akregator
{
class ArticleViewerWidget;
class AKREGATOR_EXPORT MainFrame : public Frame
{
    Q_OBJECT

public:
    explicit MainFrame(QWidget *parent, QWidget *widget);
    ~MainFrame() override;

    [[nodiscard]] QUrl url() const override;

    [[nodiscard]] bool openUrl(const OpenUrlRequest &) override
    {
        return false;
    }

    void setArticleViewer(Akregator::ArticleViewerWidget *articleViewer);

    [[nodiscard]] qreal zoomFactor() const override;

public Q_SLOTS:
    void slotCopyLinkAsInFrame(int frameId);
    void slotSaveLinkAsInFrame(int frameId);
    void slotZoomChangeInFrame(int frameId, qreal value);
    void slotCopyInFrame(int frameId);
    void slotPrintInFrame(int frameId);
    void slotPrintPreviewInFrame(int frameId);
    void slotTextToSpeechInFrame(int frameId);
    void slotFindTextInFrame(int frameId);
    void slotCopyImageLocationInFrame(int frameId);
    void slotSaveImageOnDiskInFrame(int frameId);
    void slotMute(int frameId, bool mute);

private:
    Akregator::ArticleViewerWidget *mArticleViewer = nullptr;
};
} // namespace Akregator
