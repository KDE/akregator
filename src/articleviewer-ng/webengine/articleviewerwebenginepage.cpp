/*
   SPDX-FileCopyrightText: 2015-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "articleviewerwebenginepage.h"
#include "actionmanager.h"
#include <QWebEngineSettings>
#include <WebEngineViewer/ZoomActionMenu>

#include "akregatorconfig.h"

using namespace Akregator;

ArticleViewerWebEnginePage::ArticleViewerWebEnginePage(QObject *parent)
    : WebEngineViewer::WebEnginePage(parent)
{
    settings()->setAttribute(QWebEngineSettings::AutoLoadImages, true);

    settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    settings()->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    // settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    settings()->setAttribute(QWebEngineSettings::XSSAuditingEnabled, false);
    settings()->setAttribute(QWebEngineSettings::NavigateOnDropEnabled, false);

    connect(this, &QWebEnginePage::permissionRequested, this, &ArticleViewerWebEnginePage::slotFeaturePermissionRequested);
    connect(this, &QWebEnginePage::loadProgress, this, &ArticleViewerWebEnginePage::onLoadProgress);
}

ArticleViewerWebEnginePage::~ArticleViewerWebEnginePage() = default;

bool ArticleViewerWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(type)
    if (url.scheme() == QLatin1StringView("data")) {
        return true;
    }
    if (isMainFrame && type == NavigationTypeLinkClicked) {
        Q_EMIT urlClicked(url);
        return false;
    }
    return true;
}

void ArticleViewerWebEnginePage::slotFeaturePermissionRequested(QWebEnginePermission feature)
{
    // Denied all permissions.
    feature.deny();
}

void ArticleViewerWebEnginePage::onLoadProgress()
{
    double newZoom = ActionManager::getInstance()->zoomActionMenu()->zoomFactor() / 100;
    if (zoomFactor() != newZoom) {
        setZoomFactor(newZoom);
    }
}

#include "moc_articleviewerwebenginepage.cpp"
