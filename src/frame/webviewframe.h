/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef WEBVIEWFRAME_H
#define WEBVIEWFRAME_H

#include "articleviewer-ng/articleviewerwidgetng.h"
#include "frame.h"
#include "akregatorpart_export.h"
#include <QObject>

namespace Akregator
{
class AKREGATORPART_EXPORT WebViewFrame : public Frame
{
    Q_OBJECT
public:
    explicit WebViewFrame(KActionCollection *ac, QWidget *parent = Q_NULLPTR);
    ~WebViewFrame();

    KParts::ReadOnlyPart *part() const Q_DECL_OVERRIDE;
    QUrl url() const Q_DECL_OVERRIDE;
    bool openUrl(const OpenUrlRequest &request) Q_DECL_OVERRIDE;
    void loadConfig(const KConfigGroup &, const QString &) Q_DECL_OVERRIDE;
    void saveConfig(KConfigGroup &, const QString &) Q_DECL_OVERRIDE;

    bool canGoForward() const Q_DECL_OVERRIDE;
    bool canGoBack() const Q_DECL_OVERRIDE;

    qreal zoomFactor() const Q_DECL_OVERRIDE;
    bool zoomTextOnlyInFrame() const Q_DECL_OVERRIDE;

public Q_SLOTS:
    void slotHistoryForward() Q_DECL_OVERRIDE;
    void slotHistoryBack() Q_DECL_OVERRIDE;
    void slotReload() Q_DECL_OVERRIDE;
    void slotStop() Q_DECL_OVERRIDE;

    void slotZoomChangeInFrame(int frameId, qreal value);
    void slotZoomTextOnlyInFrame(int frameId, bool textOnlyInFrame);
    void slotCopyInFrame(int frameId);
    void slotPrintInFrame(int frameId);
    void slotPrintPreviewInFrame(int frameId);
    void slotFindTextInFrame(int frameId);
    void slotTextToSpeechInFrame(int frameId);
    void slotSaveLinkAsInFrame(int frameId);
    void slotCopyLinkAsInFrame(int frameId);
    void slotSaveImageOnDiskInFrame(int frameId);
    void slotCopyImageLocationInFrame(int frameId);
    void slotBlockImageInFrame(int frameId);
private Q_SLOTS:
    void slotTitleChanged(const QString &title);
    void slotProgressChanged(int progress);
    void slotLoadStarted();
    void slotLoadFinished();
private:
    Akregator::ArticleViewerWidgetNg *mArticleViewerWidgetNg;

};
}

#endif // WEBVIEWFRAME_H
