/*
   Copyright (C) 2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef WEBENGINEFRAME_H
#define WEBENGINEFRAME_H

#include "frame.h"
#include "akregatorpart_export.h"
#include <QObject>

namespace Akregator {
class ArticleViewerWebEngineWidgetNg;
class AKREGATORPART_EXPORT WebEngineFrame : public Frame
{
    Q_OBJECT
public:
    explicit WebEngineFrame(KActionCollection *ac, QWidget *parent = nullptr);
    ~WebEngineFrame();

    QUrl url() const override;
    bool openUrl(const OpenUrlRequest &request) override;
    void loadConfig(const KConfigGroup &, const QString &) override;
    bool saveConfig(KConfigGroup &, const QString &) override;

    qreal zoomFactor() const override;

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
    Akregator::ArticleViewerWebEngineWidgetNg *mArticleViewerWidgetNg;
};
}

#endif
