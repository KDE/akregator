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
#include "actionmanager.h"
#include "actions.h"
#include "urlhandlermanager.h"
#include "akregatorconfig.h"
#include <KActionCollection>
#include <KParts/BrowserRun>
#include <KMessageBox>
#include <KAboutData>
#include <KPIMTextEdit/TextToSpeech>
#include <KLocalizedString>
#include <MessageViewer/WebViewAccessKey>
#include <MessageViewer/WebPage>
#include <KActionMenu>
#include <QWebSettings>
#include <QClipboard>
#include <QApplication>
#include <MessageViewer/AdBlockManager>

#include <QMenu>
#include <QWebFrame>
#include <QMouseEvent>
#include <grantleethememanager.h>
#include <openurlrequest.h>
#include <QWebHistory>
#include <KConfigGroup>
#include <kpimprintpreviewdialog.h>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

#include <KIO/KUriFilterSearchProviderActions>
using namespace Akregator;

ArticleViewerNg::ArticleViewerNg(KActionCollection *ac, QWidget *parent)
    : KWebView(parent, false),
      mActionCollection(ac),
      mLastButtonClicked(LeftButton)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
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
    connect(this, &ArticleViewerNg::showContextMenu, this, &ArticleViewerNg::slotShowContextMenu);
    mShareServiceManager = new PimCommon::ShareServiceUrlManager(this);
    connect(mShareServiceManager, &PimCommon::ShareServiceUrlManager::serviceUrlSelected, this, &ArticleViewerNg::slotServiceUrlSelected);
    mWebShortcutMenuManager = new KIO::KUriFilterSearchProviderActions(this);
}

ArticleViewerNg::~ArticleViewerNg()
{
    disconnect(this, &QWebView::loadFinished, this, &ArticleViewerNg::slotLoadFinished);
}

void ArticleViewerNg::slotSaveLinkAs()
{
    QUrl tmp(mCurrentUrl);

    if (tmp.fileName().isEmpty()) {
        tmp = tmp.adjusted(QUrl::RemoveFilename);
        tmp.setPath(tmp.path() + QLatin1String("index.html"));
    }
    KParts::BrowserRun::simpleSave(tmp, tmp.fileName());
}

void ArticleViewerNg::slotSaveImageOnDiskInFrame()
{
    //TODO
}

void ArticleViewerNg::slotCopyImageLocationInFrame()
{
    //TODO
}

void ArticleViewerNg::slotCopyLinkAddress()
{
    if (mCurrentUrl.isEmpty()) {
        return;
    }
    QClipboard *cb = QApplication::clipboard();
    cb->setText(mCurrentUrl.toString(), QClipboard::Clipboard);
    // don't set url to selection as it's a no-no according to a fd.o spec
    // which spec? Nobody seems to care (tested Firefox (3.5.10) Konqueror,and KMail (4.2.3)), so I re-enable the following line unless someone gives
    // a good reason to remove it again (bug 183022) --Frank
    cb->setText(mCurrentUrl.toString(), QClipboard::Selection);
}

void ArticleViewerNg::slotShowContextMenu(const QPoint &pos)
{
    displayContextMenu(pos);
}

void ArticleViewerNg::slotPrintPreview()
{
    PimCommon::KPimPrintPreviewDialog previewdlg(this);
    connect(&previewdlg, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter * printer) {
        print(printer);
    });
    previewdlg.exec();
}

void ArticleViewerNg::slotPrint()
{
    QPrinter printer;

    QScopedPointer<QPrintDialog> dlg(new QPrintDialog(&printer));

    if (dlg && dlg->exec() == QDialog::Accepted) {
        print(&printer);
    }
}

void ArticleViewerNg::slotCopy()
{
    triggerPageAction(QWebPage::Copy);
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

QVariantHash ArticleViewerNg::introductionData() const
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

void ArticleViewerNg::displayContextMenu(const QPoint &pos)
{
    mContextMenuHitResult = page()->mainFrame()->hitTestContent(pos);
    mCurrentUrl = mContextMenuHitResult.linkUrl();
    if (URLHandlerManager::instance()->handleContextMenuRequest(mCurrentUrl, mapToGlobal(pos), this)) {
        return;
    }

    QMenu popup(this);
    const bool contentSelected = mContextMenuHitResult.isContentSelected();
    if (!mCurrentUrl.isEmpty() && !contentSelected) {
        popup.addAction(createOpenLinkInNewTabAction(mCurrentUrl, this, SLOT(slotOpenLinkInForegroundTab()), &popup));
        popup.addAction(createOpenLinkInExternalBrowserAction(mCurrentUrl, this, SLOT(slotOpenLinkInBrowser()), &popup));
        popup.addSeparator();
        popup.addAction(mActionCollection->action(QStringLiteral("savelinkas")));
        popup.addAction(mActionCollection->action(QStringLiteral("copylinkaddress")));
        if (!mContextMenuHitResult.imageUrl().isEmpty()) {
            popup.addSeparator();
            popup.addAction(mActionCollection->action(QStringLiteral("copy_image_location")));
            popup.addAction(mActionCollection->action(QStringLiteral("saveas_imageurl")));

#if 0
            popup.addAction(mMsgView->shareImage());
            if (mMsgView->adblockEnabled()) {
                popup.addSeparator();
                popup.addAction(mMsgView->blockImage());
            }
#endif
            //TODO
        }
        popup.addSeparator();
        popup.addAction(mShareServiceManager->menu());
    } else {
        if (contentSelected) {
            popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_copy")));
            popup.addSeparator();
        }
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_print")));
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_printpreview")));
    }
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("find_in_messages")));
    if (KPIMTextEdit::TextToSpeech::self()->isReady()) {
        popup.addSeparator();
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("speak_text")));
    }
    popup.exec(mapToGlobal(pos));
}

void ArticleViewerNg::slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type)
{
    const QUrl url = mShareServiceManager->generateServiceUrl(mCurrentUrl.url(), QString(), type);
    mShareServiceManager->openUrl(url);
}

void ArticleViewerNg::slotLinkHovered(const QString &link, const QString &title, const QString &textContent)
{
    Q_UNUSED(title);
    Q_UNUSED(textContent);
    QUrl url(linkOrImageUrlAt(QCursor::pos()));
    QString msg = URLHandlerManager::instance()->statusBarMessage(url, this);
    if (msg.isEmpty()) {
        msg = link;
    }

    Q_EMIT showStatusBarMessage(msg);
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

void ArticleViewerNg::disableIntroduction()
{
    KGuiItem yesButton(KStandardGuiItem::yes());
    yesButton.setText(i18n("Disable"));
    KGuiItem noButton(KStandardGuiItem::no());
    noButton.setText(i18n("Keep Enabled"));
    if (KMessageBox::questionYesNo(this, i18n("Are you sure you want to disable this introduction page?"),
                                   i18n("Disable Introduction Page"), yesButton, noButton) == KMessageBox::Yes) {
        Settings::self()->setDisableIntroduction(true);
        Settings::self()->save();
    }
}

void ArticleViewerNg::setArticleAction(ArticleViewerNg::ArticleAction type, const QString &articleId)
{
    Q_EMIT articleAction(type, articleId);
}

void ArticleViewerNg::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton) {
        Q_EMIT showContextMenu(event->pos());
        mLastButtonClicked = RightButton;
    } else if (event->button() & Qt::MiddleButton) {
        mLastButtonClicked = MiddleButton;
    } else if (event->button() & Qt::LeftButton) {
        mLastButtonClicked = LeftButton;
    }
    QWebView::mouseReleaseEvent(event);
}

void ArticleViewerNg::slotLinkClicked(const QUrl &url)
{
    if (URLHandlerManager::instance()->handleClick(url, this)) {
        return;
    }

    mCurrentUrl = url;
    OpenUrlRequest req(mCurrentUrl);
    if (mLastButtonClicked == LeftButton) {
        switch (Settings::lMBBehaviour()) {
        case Settings::EnumLMBBehaviour::OpenInExternalBrowser:
            req.setOptions(OpenUrlRequest::ExternalBrowser);
            break;
        case Settings::EnumLMBBehaviour::OpenInBackground:
            req.setOpenInBackground(true);
            req.setOptions(OpenUrlRequest::NewTab);
            break;
        default:
            break;
        }
    } else if (mLastButtonClicked == MiddleButton) {
        switch (Settings::mMBBehaviour()) {
        case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
            req.setOptions(OpenUrlRequest::ExternalBrowser);
            break;
        case Settings::EnumMMBBehaviour::OpenInBackground:
            req.setOpenInBackground(true);
            req.setOptions(OpenUrlRequest::NewTab);
            break;
        default:
            break;
        }
    }
    Q_EMIT signalOpenUrlRequest(req);
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

QUrl ArticleViewerNg::linkOrImageUrlAt(const QPoint &global) const
{
    const QPoint local = page()->view()->mapFromGlobal(global);
    const QWebHitTestResult hit = page()->currentFrame()->hitTestContent(local);
    if (!hit.linkUrl().isEmpty()) {
        return hit.linkUrl();
    } else if (!hit.imageUrl().isEmpty()) {
        return hit.imageUrl();
    } else {
        return QUrl();
    }
}

bool ArticleViewerNg::zoomTextOnlyInFrame() const
{
    return settings()->testAttribute(QWebSettings::ZoomTextOnly);
}

void ArticleViewerNg::slotZoomTextOnlyInFrame(bool textOnlyInFrame)
{
    settings()->setAttribute(QWebSettings::ZoomTextOnly, textOnlyInFrame);
}

void ArticleViewerNg::slotBlockImage()
{
    if (mCurrentUrl.isEmpty()) {
        return;
    }
    MessageViewer::AdBlockManager::self()->addCustomRule(mCurrentUrl.url(), true);
}
