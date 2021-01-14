/*
   SPDX-FileCopyrightText: 2015-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "articleviewerwebenginepage.h"
#include <QWebEngineSettings>
#include <actionmanager.h>
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
    //settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
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
    Q_UNUSED(type)
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
    double newZoom = ActionManager::getInstance()->zoomActionMenu()->zoomFactor() / 100;
    if (zoomFactor() != newZoom) {
        setZoomFactor(newZoom);
    }
}
