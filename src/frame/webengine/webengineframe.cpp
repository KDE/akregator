/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "webengineframe.h"
#include "akrwebengineviewer.h"
#include "articleviewer-ng/webengine/articleviewerwebenginewidgetng.h"

#include <KIO/FavIconRequestJob>

#include <QVBoxLayout>

using namespace Akregator;

WebEngineFrame::WebEngineFrame(KActionCollection *ac, QWidget *parent)
    : Frame(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins({});
    setRemovable(true);
    auto *viewer = new Akregator::AkrWebEngineViewer(ac, this);
    mArticleViewerWidgetNg = new Akregator::ArticleViewerWebEngineWidgetNg(viewer, ac, this);

    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::titleChanged,
            this, &WebEngineFrame::slotTitleChanged);

    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::loadProgress,
            this, &WebEngineFrame::slotProgressChanged);

    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::signalOpenUrlRequest,
            this, &WebEngineFrame::signalOpenUrlRequest);

    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::loadStarted,
            this, &WebEngineFrame::slotLoadStarted);

    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::loadFinished,
            this, &WebEngineFrame::slotLoadFinished);

    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::showStatusBarMessage,
            this, &WebEngineFrame::showStatusBarMessage);

    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerWebEngine::webPageMutedOrAudibleChanged,
            this, &WebEngineFrame::slotWebPageMutedOrAudibleChanged);

    layout->addWidget(mArticleViewerWidgetNg);
}

WebEngineFrame::~WebEngineFrame()
{
}

void WebEngineFrame::slotWebPageMutedOrAudibleChanged(bool isAudioMuted, bool wasRecentlyAudible)
{
    Q_EMIT webPageMutedOrAudibleChanged(this, isAudioMuted, wasRecentlyAudible);
}

void WebEngineFrame::slotLoadFinished()
{
    Q_EMIT signalCompleted(this);
}

void WebEngineFrame::slotLoadStarted()
{
    Q_EMIT signalStarted(this);
}

void WebEngineFrame::slotProgressChanged(int progress)
{
    Q_EMIT signalLoadingProgress(this, progress);
}

void WebEngineFrame::slotTitleChanged(const QString &title)
{
    slotSetTitle(title);
}

QUrl WebEngineFrame::url() const
{
    return mArticleViewerWidgetNg->articleViewerNg()->url();
}

void WebEngineFrame::loadUrl(const QUrl &url)
{
    auto *job = new KIO::FavIconRequestJob(url);
    connect(job, &KIO::FavIconRequestJob::result, this, [job, this](KJob *) {
        if (!job->error()) {
            Q_EMIT signalIconChanged(this, QIcon(job->iconFile()));
        }
    });

    mArticleViewerWidgetNg->articleViewerNg()->load(url);
}

bool WebEngineFrame::openUrl(const OpenUrlRequest &request)
{
    const QUrl url = request.url();
    loadUrl(url);
    return true;
}

void WebEngineFrame::loadConfig(const KConfigGroup &config, const QString &prefix)
{
    const QString url = config.readEntry(QStringLiteral("url").prepend(prefix), QString());
    const qreal zf = config.readEntry(QStringLiteral("zoom").prepend(prefix), 1.0);
    mArticleViewerWidgetNg->articleViewerNg()->setZoomFactor(zf);
    loadUrl(QUrl(url));
}

bool WebEngineFrame::saveConfig(KConfigGroup &config, const QString &prefix)
{
    const QString urlPath(url().url());
    if (urlPath != QLatin1String("about:blank")) {
        config.writeEntry(QStringLiteral("url").prepend(prefix), urlPath);
        config.writeEntry(QStringLiteral("zoom").prepend(prefix), zoomFactor());
        return true;
    } else {
        return false;
    }
}

void WebEngineFrame::slotCopyInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotCopy();
}

void WebEngineFrame::slotPrintInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->slotPrint();
}

void WebEngineFrame::slotPrintPreviewInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->slotPrintPreview();
}

void WebEngineFrame::slotFindTextInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->slotFind();
}

void WebEngineFrame::slotTextToSpeechInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->slotSpeakText();
}

void WebEngineFrame::slotZoomChangeInFrame(int frameId, qreal value)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->setZoomFactor(value);
}

void WebEngineFrame::slotReload()
{
    mArticleViewerWidgetNg->articleViewerNg()->reload();
}

void WebEngineFrame::slotStop()
{
    mArticleViewerWidgetNg->articleViewerNg()->stop();
}

qreal WebEngineFrame::zoomFactor() const
{
    return mArticleViewerWidgetNg->articleViewerNg()->zoomFactor();
}

void WebEngineFrame::slotSaveLinkAsInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotSaveLinkAs();
}

void WebEngineFrame::slotCopyLinkAsInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotCopyLinkAddress();
}

void WebEngineFrame::slotSaveImageOnDiskInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotSaveImageOnDiskInFrame();
}

void WebEngineFrame::slotCopyImageLocationInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotCopyImageLocationInFrame();
}

void WebEngineFrame::slotMute(int frameId, bool mute)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotMute(mute);
}
