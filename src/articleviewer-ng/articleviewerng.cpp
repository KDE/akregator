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
#include <KMessageBox>
#include <KAboutData>
#include <KLocalizedString>
#include <MessageViewer/WebViewAccessKey>
#include <MessageViewer/WebPage>
#include <QWebSettings>
#include <QDesktopServices>

#include <QMenu>
#include <QWebFrame>
#include <QMouseEvent>
#include <grantleethememanager.h>
#include <openurlrequest.h>
#include <QWebHistory>
#include <KConfigGroup>
using namespace Akregator;
namespace
{
qreal zoomBy()
{
    return 20;
}
}

ArticleViewerNg::ArticleViewerNg(KActionCollection *ac, QWidget *parent)
    : KWebView(parent, false),
      mZoomFactor(100),
      mActionCollection(ac)
{
    setPage(new MessageViewer::WebPage(this));
    mWebViewAccessKey = new MessageViewer::WebViewAccessKey(this, this);
    mWebViewAccessKey->setActionCollection(mActionCollection);

    settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    settings()->setAttribute(QWebSettings::JavaEnabled, false);
    settings()->setAttribute(QWebSettings::PluginsEnabled, false);
    settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
    settings()->setAttribute(QWebSettings::AutoLoadImages, true);

    connect(this, &QWebView::loadStarted, this, &ArticleViewerNg::slotLoadStarted);
    connect(this, &QWebView::loadFinished, this, &ArticleViewerNg::slotLoadFinished);
    connect(page(), &QWebPage::scrollRequested, mWebViewAccessKey, &MessageViewer::WebViewAccessKey::hideAccessKeys);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(page(), &QWebPage::linkHovered, this, &ArticleViewerNg::slotLinkHovered);
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
    QVariantHash data;
    data[QStringLiteral("icon")] = QStringLiteral("akregator");
    data[QStringLiteral("name")] = i18n("Akregator");
    data[QStringLiteral("subtitle")] = i18n("Akregator is a KDE news feed reader.");
    data[QStringLiteral("version")] = KAboutData::applicationData().version();
    return data;
}

void ArticleViewerNg::showAboutPage()
{
    paintAboutScreen(QStringLiteral("introduction_akregator.html"), introductionData());
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
    }
    popup.exec(mapToGlobal(event->pos()));
}

void ArticleViewerNg::setZoomIn()
{
    if (mZoomFactor >= 300) {
        return;
    }
    mZoomFactor += zoomBy();
    if (mZoomFactor > 300) {
        mZoomFactor = 300;
    }
    setZoomFactor(mZoomFactor / 100.0);
}

void ArticleViewerNg::setZoomOut()
{
    if (mZoomFactor <= 10) {
        return;
    }
    mZoomFactor -= zoomBy();
    if (mZoomFactor < 10) {
        mZoomFactor = 10;
    }
    setZoomFactor(mZoomFactor / 100.0);
}

void ArticleViewerNg::slotLinkHovered(const QString &link, const QString &title, const QString &textContent)
{
    Q_EMIT showStatusBarMessage(link);
    //TODO
}

void ArticleViewerNg::keyReleaseEvent(QKeyEvent *e)
{
    if (Settings::self()->accessKeyEnabled()) {
        mWebViewAccessKey->keyReleaseEvent(e);
    }
    KWebView::keyReleaseEvent(e);
}

void ArticleViewerNg::keyPressEvent(QKeyEvent *e)
{
    if (e && hasFocus()) {
        if (Settings::self()->accessKeyEnabled()) {
            mWebViewAccessKey->keyPressEvent(e);
        }
    }
    KWebView::keyPressEvent(e);
}

void ArticleViewerNg::wheelEvent(QWheelEvent *e)
{
    if (Settings::self()->accessKeyEnabled()) {
        mWebViewAccessKey->wheelEvent(e);
    }
    KWebView::wheelEvent(e);
}

void ArticleViewerNg::resizeEvent(QResizeEvent *e)
{
    if (Settings::self()->accessKeyEnabled()) {
        mWebViewAccessKey->resizeEvent(e);
    }
    KWebView::resizeEvent(e);
}

void ArticleViewerNg::slotLinkClicked(const QUrl &url)
{
    if (url.toString() == QLatin1String("config:/disable_introduction")) {
        KGuiItem yesButton(KStandardGuiItem::yes());
        yesButton.setText(i18n("Disable"));
        KGuiItem noButton(KStandardGuiItem::no());
        noButton.setText(i18n("Keep Enabled"));
        if (KMessageBox::questionYesNo(this, i18n("Are you sure you want to disable this introduction page?"), i18n("Disable Introduction Page"), yesButton, noButton) == KMessageBox::Yes) {
            Settings::self()->setDisableIntroduction(true);
            Settings::self()->save();
        }
        return;
    } else if (url.scheme() == QLatin1String("mailto")) {
        QDesktopServices::openUrl(url);
        return;
    }

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
