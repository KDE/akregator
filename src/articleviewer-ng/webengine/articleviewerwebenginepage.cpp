/*
   Copyright (C) 2015-2018 Montel Laurent <montel@kde.org>

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

#include "articleviewerwebenginepage.h"
#include <QWebEngineSettings>
#include <QWebEngineProfile>

#include "akregatorconfig.h"

using namespace Akregator;

ArticleViewerWebEnginePage::ArticleViewerWebEnginePage(QWebEngineProfile *profile, QObject *parent)
    : WebEngineViewer::WebEnginePage(profile, parent)
{
    settings()->setAttribute(QWebEngineSettings::AutoLoadImages, true);

    settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    settings()->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    settings()->setAttribute(QWebEngineSettings::XSSAuditingEnabled, false);

    connect(this, &QWebEnginePage::featurePermissionRequested,
            this, &ArticleViewerWebEnginePage::slotFeaturePermissionRequested);
    connect(this, &QWebEnginePage::loadProgress,
            this, &ArticleViewerWebEnginePage::onLoadProgress);
}

ArticleViewerWebEnginePage::~ArticleViewerWebEnginePage()
{
}

bool ArticleViewerWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(type);
    if (url.scheme() == QLatin1String("data")) {
        return true;
    }
    if (isMainFrame && type == NavigationTypeLinkClicked) {
        Q_EMIT urlClicked(url);
        return false;
    }
    return true;
}

void ArticleViewerWebEnginePage::slotFeaturePermissionRequested(const QUrl &url, QWebEnginePage::Feature feature)
{
    //Denied all permissions.
    setFeaturePermission(url, feature, QWebEnginePage::PermissionDeniedByUser);
}

void ArticleViewerWebEnginePage::onLoadProgress()
{
    double newZoom = static_cast<double>(Settings::zoom() / 100.0);
    if (zoomFactor() != newZoom) {
        setZoomFactor(newZoom);
    }
}
