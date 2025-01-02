/*
   SPDX-FileCopyrightText: 2015-2025 Laurent Montel <montel@kde.org>

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

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    connect(this, &QWebEnginePage::permissionRequested, this, &ArticleViewerWebEnginePage::slotFeaturePermissionRequested);
#else
    connect(this, &QWebEnginePage::featurePermissionRequested, this, &ArticleViewerWebEnginePage::slotFeaturePermissionRequested);
#endif
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
void ArticleViewerWebEnginePage::slotFeaturePermissionRequested(QWebEnginePermission feature)
#else
void ArticleViewerWebEnginePage::slotFeaturePermissionRequested(const QUrl &url, QWebEnginePage::Feature feature)
#endif
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    // Denied all permissions.
    feature.deny();
#else
    setFeaturePermission(url, feature, QWebEnginePage::PermissionDeniedByUser);
#endif
}

void ArticleViewerWebEnginePage::onLoadProgress()
{
    double newZoom = ActionManager::getInstance()->zoomActionMenu()->zoomFactor() / 100;
    if (zoomFactor() != newZoom) {
        setZoomFactor(newZoom);
    }
}

#include "moc_articleviewerwebenginepage.cpp"
