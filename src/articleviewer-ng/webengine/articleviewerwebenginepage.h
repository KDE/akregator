/*
   Copyright (c) 2015-2020 Laurent Montel <montel@kde.org>

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

#ifndef ARTICLEVIEWERWEBENGINEPAGE_H
#define ARTICLEVIEWERWEBENGINEPAGE_H

#include <WebEngineViewer/WebEnginePage>
namespace Akregator {
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
