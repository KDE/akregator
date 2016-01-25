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

#include "openurlrequest.h"
#include "webviewframe.h"
#include "webviewer.h"
#include <QVBoxLayout>
#include <QAction>

using namespace Akregator;

WebViewFrame::WebViewFrame(KActionCollection *ac, QWidget *parent)
    : Frame(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setRemovable(true);
    Akregator::WebViewer *viewer = new Akregator::WebViewer(ac, this);
    mArticleViewerWidgetNg = new Akregator::ArticleViewerWidgetNg(viewer, ac, this);

    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerNg::titleChanged, this, &WebViewFrame::slotTitleChanged);
    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerNg::loadProgress, this, &WebViewFrame::slotProgressChanged);
    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerNg::signalOpenUrlRequest, this, &WebViewFrame::signalOpenUrlRequest);
    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerNg::loadStarted, this, &WebViewFrame::slotLoadStarted);
    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerNg::loadFinished, this, &WebViewFrame::slotLoadFinished);
    connect(mArticleViewerWidgetNg->articleViewerNg(), &ArticleViewerNg::showStatusBarMessage, this, &WebViewFrame::showStatusBarMessage);
    layout->addWidget(mArticleViewerWidgetNg);
}

WebViewFrame::~WebViewFrame()
{
}

void WebViewFrame::slotLoadFinished()
{
    Q_EMIT signalCompleted(this);
}

void WebViewFrame::slotLoadStarted()
{
    Q_EMIT signalStarted(this);
}

void WebViewFrame::slotProgressChanged(int progress)
{
    Q_EMIT signalLoadingProgress(this, progress);
}

void WebViewFrame::slotTitleChanged(const QString &title)
{
    Q_EMIT signalTitleChanged(this, title);
}

KParts::ReadOnlyPart *WebViewFrame::part() const
{
    return Q_NULLPTR;
}

QUrl WebViewFrame::url() const
{
    return mArticleViewerWidgetNg->articleViewerNg()->url();
}

bool WebViewFrame::openUrl(const OpenUrlRequest &request)
{
    //TODO
    mArticleViewerWidgetNg->articleViewerNg()->load(request.url());
    return true;
}

void WebViewFrame::loadConfig(const KConfigGroup &config, const QString &prefix)
{
    const QString url = config.readEntry(QStringLiteral("url").prepend(prefix), QString());
    const qreal zf = config.readEntry(QStringLiteral("zoom").prepend(prefix), 1.0);
    const bool onlyZoomFont = config.readEntry(QStringLiteral("onlyZoomFont").prepend(prefix), false);
    OpenUrlRequest req(url);
    KParts::OpenUrlArguments args;
    req.setArgs(args);
    openUrl(req);
    mArticleViewerWidgetNg->articleViewerNg()->setZoomFactor(zf);
    mArticleViewerWidgetNg->articleViewerNg()->settings()->setAttribute(QWebSettings::ZoomTextOnly, onlyZoomFont);
}

void WebViewFrame::saveConfig(KConfigGroup &config, const QString &prefix)
{
    config.writeEntry(QStringLiteral("url").prepend(prefix), url().url());
    config.writeEntry(QStringLiteral("zoom").prepend(prefix), mArticleViewerWidgetNg->articleViewerNg()->zoomFactor());
    config.writeEntry(QStringLiteral("onlyZoomFont").prepend(prefix), mArticleViewerWidgetNg->articleViewerNg()->settings()->testAttribute(QWebSettings::ZoomTextOnly));
}

void WebViewFrame::setFavicon(const QIcon &icon)
{
    Q_EMIT signalIconChanged(this, icon);
}

void WebViewFrame::slotCopyInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotCopy();
}

void WebViewFrame::slotPrintInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotPrint();
}

void WebViewFrame::slotPrintPreviewInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotPrintPreview();
}

void WebViewFrame::slotFindTextInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->slotFind();
}

void WebViewFrame::slotTextToSpeechInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->slotSpeakText();
}

void WebViewFrame::slotZoomChangeInFrame(int frameId, qreal value)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->setZoomFactor(value);
}

void WebViewFrame::slotZoomTextOnlyInFrame(int frameId, bool textOnlyInFrame)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotZoomTextOnlyInFrame(textOnlyInFrame);
}

void WebViewFrame::slotHistoryForward()
{
    mArticleViewerWidgetNg->articleViewerNg()->pageAction(QWebPage::Forward)->trigger();
}

void WebViewFrame::slotHistoryBack()
{
    mArticleViewerWidgetNg->articleViewerNg()->pageAction(QWebPage::Back)->trigger();
}

void WebViewFrame::slotReload()
{
    mArticleViewerWidgetNg->articleViewerNg()->reload();
}

void WebViewFrame::slotStop()
{
    mArticleViewerWidgetNg->articleViewerNg()->stop();
}

bool WebViewFrame::canGoForward() const
{
    return mArticleViewerWidgetNg->articleViewerNg()->canGoForward();
}

bool WebViewFrame::canGoBack() const
{
    return mArticleViewerWidgetNg->articleViewerNg()->canGoBack();
}

qreal WebViewFrame::zoomFactor() const
{
    return mArticleViewerWidgetNg->articleViewerNg()->zoomFactor();
}

bool WebViewFrame::zoomTextOnlyInFrame() const
{
    return mArticleViewerWidgetNg->articleViewerNg()->zoomTextOnlyInFrame();
}

void WebViewFrame::slotSaveLinkAsInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotSaveLinkAs();
}

void WebViewFrame::slotCopyLinkAsInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotCopyLinkAddress();
}

void WebViewFrame::slotSaveImageOnDiskInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotSaveImageOnDiskInFrame();
}

void WebViewFrame::slotCopyImageLocationInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotCopyImageLocationInFrame();
}

void WebViewFrame::slotBlockImagesInFrame(int frameId)
{
    if (frameId != id()) {
        return;
    }
    mArticleViewerWidgetNg->articleViewerNg()->slotBlockImage();
}
