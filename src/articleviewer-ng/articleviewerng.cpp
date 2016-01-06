/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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


#include "articleviewerng.h"
#include "akregator_debug.h"
#include "../actions.h"
#include "../actionmanager.h"
#include "akregatorconfig.h"
#include <KActionCollection>
#include <MessageViewer/WebViewAccessKey>
#include <MessageViewer/WebPage>
#include <QWebSettings>

#include <QMenu>
#include <QWebFrame>
#include <QMouseEvent>
#include <grantleethememanager.h>
#include <openurlrequest.h>
#include <QWebHistory>
using namespace Akregator;

ArticleViewerNg::ArticleViewerNg(KActionCollection *ac, QWidget *parent)
    : KWebView(parent, false),
      mActionCollection(ac)
{
    setPage(new MessageViewer::WebPage(this));
    mWebViewAccessKey = new MessageViewer::WebViewAccessKey(this, this);
    mWebViewAccessKey->setActionCollection(mActionCollection);

    //setZoomFactor(100);
    settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    settings()->setAttribute(QWebSettings::JavaEnabled, false);
    settings()->setAttribute(QWebSettings::PluginsEnabled, false);
    settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
    settings()->setAttribute(QWebSettings::AutoLoadImages, true);

    connect(this, &QWebView::loadStarted, this, &ArticleViewerNg::slotLoadStarted);
    connect(this, &QWebView::loadFinished, this, &ArticleViewerNg::slotLoadFinished);
    connect(page(), &QWebPage::scrollRequested, mWebViewAccessKey, &MessageViewer::WebViewAccessKey::hideAccessKeys);
    //TODO make it customizable
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this, &ArticleViewerNg::linkClicked, this, &ArticleViewerNg::slotLinkClicked);

}

ArticleViewerNg::~ArticleViewerNg()
{
    disconnect(this, &QWebView::loadFinished, this, &ArticleViewerNg::slotLoadFinished);
}

void ArticleViewerNg::slotLoadFinished()
{
    unsetCursor();
}

void ArticleViewerNg::slotLoadStarted()
{
    mWebViewAccessKey->hideAccessKeys();
    setCursor(Qt::WaitCursor);
}

QVariantHash ArticleViewerNg::introductionData()
{
    return QVariantHash();
}

void ArticleViewerNg::showAboutPage()
{
    paintAboutScreen(QStringLiteral("introduction_akregator.html"), introductionData());
    //TODO call paintAboutScreen
}

bool ArticleViewerNg::canGoForward() const
{
    QWebHistory *history = page()->history();
    return history->canGoForward();
}

bool ArticleViewerNg::canGoBack() const
{
    QWebHistory *history = page()->history();
    return history->canGoBack();
}

void ArticleViewerNg::paintAboutScreen(const QString &templateName, const QVariantHash &data)
{
    GrantleeTheme::ThemeManager manager(QStringLiteral("splashPage"),
                                        QStringLiteral("splash.theme"),
                                        Q_NULLPTR,
                                        QStringLiteral("messageviewer/about/"));
    GrantleeTheme::Theme theme = manager.theme(QStringLiteral("default"));
    if (!theme.isValid()) {
        qCDebug(AKREGATOR_LOG) << "Theme error: failed to find splash theme";
    } else {
        setHtml(theme.render(templateName, data),
                            QUrl::fromLocalFile(theme.absolutePath() + QLatin1Char('/')));
    }
}

void ArticleViewerNg::contextMenuEvent(QContextMenuEvent *event)
{
    mContextMenuHitResult = page()->mainFrame()->hitTestContent(event->pos());
    QMenu popup(this);
    mCurrentUrl = mContextMenuHitResult.linkUrl();
    const bool contentSelected = mContextMenuHitResult.isContentSelected();
    if (!mCurrentUrl.isEmpty() && !contentSelected) {
        popup.addAction(createOpenLinkInNewTabAction(mCurrentUrl, this, SLOT(slotOpenLinkInForegroundTab()), &popup));
        popup.addAction(createOpenLinkInExternalBrowserAction(mCurrentUrl, this, SLOT(slotOpenLinkInBrowser()), &popup));
        popup.addSeparator();
        popup.addAction(mActionCollection->action(QStringLiteral("savelinkas")));
        popup.addAction(mActionCollection->action(QStringLiteral("copylinkaddress")));
    } else {
        if (contentSelected) {
            popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_copy")));
            popup.addSeparator();
        }
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_print")));
        popup.addSeparator();
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("inc_font_sizes")));
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("dec_font_sizes")));
    }
    popup.exec(mapToGlobal(event->pos()));
}

void ArticleViewerNg::keyReleaseEvent(QKeyEvent *e)
{
    if (/*MessageViewer::MessageViewerSettings::self()->accessKeyEnabled()*/1) {
        mWebViewAccessKey->keyReleaseEvent(e);
    }
    KWebView::keyReleaseEvent(e);
}

void ArticleViewerNg::keyPressEvent(QKeyEvent *e)
{
    if (e && hasFocus()) {
        if (/*MessageViewer::MessageViewerSettings::self()->accessKeyEnabled()*/1) {
            mWebViewAccessKey->keyPressEvent(e);
        }
    }
    KWebView::keyPressEvent(e);
}

void ArticleViewerNg::wheelEvent(QWheelEvent *e)
{
    if (/*MessageViewer::MessageViewerSettings::self()->accessKeyEnabled()*/1) {
        mWebViewAccessKey->wheelEvent(e);
    }
    KWebView::wheelEvent(e);
}

void ArticleViewerNg::resizeEvent(QResizeEvent *e)
{
    if (/*MessageViewer::MessageViewerSettings::self()->accessKeyEnabled()*/1) {
        mWebViewAccessKey->resizeEvent(e);
    }
    KWebView::resizeEvent(e);
}

void ArticleViewerNg::slotLinkClicked(const QUrl &url)
{
    mCurrentUrl = url;
    OpenUrlRequest req(mCurrentUrl);
    req.setOptions(OpenUrlRequest::NewTab);
    Q_EMIT signalOpenUrlRequest(req);
    //TODO open it.
}

void ArticleViewerNg::slotOpenLinkInForegroundTab()
{
    OpenUrlRequest req(mCurrentUrl);
    req.setOptions(OpenUrlRequest::NewTab);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerNg::slotOpenLinkInBackgroundTab()
{
    OpenUrlRequest req(mCurrentUrl);
    req.setOptions(OpenUrlRequest::NewTab);
    req.setOpenInBackground(true);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerNg::slotOpenLinkInBrowser()
{
    OpenUrlRequest req(mCurrentUrl);
    req.setOptions(OpenUrlRequest::ExternalBrowser);
    Q_EMIT signalOpenUrlRequest(req);
}
