/*
   SPDX-FileCopyrightText: 2015-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ARTICLEVIEWERWEBENGINEPAGE_H
#define ARTICLEVIEWERWEBENGINEPAGE_H

#include <WebEngineViewer/WebEnginePage>
namespace Akregator
{
class ArticleViewerWebEnginePage : public WebEngineViewer::WebEnginePage
{
    Q_OBJECT
public:
    explicit ArticleViewerWebEnginePage(QObject *parent = nullptr);
    ~ArticleViewerWebEnginePage() override;

Q_SIGNALS:
    void urlClicked(const QUrl &url);

protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;

private:
    void slotFeaturePermissionRequested(const QUrl &url, QWebEnginePage::Feature feature);
    void onLoadProgress();
};
}
#endif // ARTICLEVIEWERWEBENGINEPAGE_H
