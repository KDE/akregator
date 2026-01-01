/*
   SPDX-FileCopyrightText: 2015-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <WebEngineViewer/WebEnginePage>
namespace Akregator
{
class ArticleViewerWebEnginePage : public WebEngineViewer::WebEnginePage
{
    Q_OBJECT
public:
    explicit ArticleViewerWebEnginePage(QObject *parent = nullptr);
    ~ArticleViewerWebEnginePage() override;

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;

private:
    void slotFeaturePermissionRequested(QWebEnginePermission feature);
    void onLoadProgress();
};
}
