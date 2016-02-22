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

#include "articleviewerwebengine.h"
#include "akregator_debug.h"
#include "articleviewerwebenginepage.h"
#include "webengine/urlhandlerwebenginemanager.h"
#include "actionmanager.h"
#include "akregatorconfig.h"
#include "actions/actions.h"
#include "urlhandler/webengine/urlhandlerwebengine.h"

#include <MessageViewer/MailNetworkAccessManager>
#include <KPIMTextEdit/TextToSpeech>
#include <KActionMenu>

#include <KParts/BrowserRun>
#include <grantleethememanager.h>
#include <KActionCollection>
#include <KLocalizedString>
#include <KAboutData>
#include <KGuiItem>
#include <openurlrequest.h>
#include <KPimPrintPreviewDialog>
#include <KMessageBox>
#include <QPrinter>
#include <QWebEngineSettings>
#include <QMouseEvent>
#include <QPrintDialog>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <MessageViewer/MailWebHitTestResult>

#include <KIO/KUriFilterSearchProviderActions>

using namespace Akregator;

ArticleViewerWebEngine::ArticleViewerWebEngine(KActionCollection *ac, QWidget *parent)
    : QWebEngineView(parent),
      mActionCollection(ac),
      mContextMenuHitResult(Q_NULLPTR),
      mLastButtonClicked(LeftButton)
{
    new MessageViewer::MailNetworkAccessManager(this);
    ArticleViewerWebEnginePage *pageEngine = new ArticleViewerWebEnginePage(this);
    setPage(pageEngine);

    connect(this, &ArticleViewerWebEngine::showContextMenu, this, &ArticleViewerWebEngine::slotShowContextMenu);

    setFocusPolicy(Qt::WheelFocus);
    connect(pageEngine, &ArticleViewerWebEnginePage::urlClicked, this, &ArticleViewerWebEngine::slotLinkClicked);

    //mWebViewAccessKey = new MessageViewer::WebViewAccessKey(this, this);
    //mWebViewAccessKey->setActionCollection(mActionCollection);

    connect(this, &ArticleViewerWebEngine::loadStarted, this, &ArticleViewerWebEngine::slotLoadStarted);
    connect(this, &ArticleViewerWebEngine::loadFinished, this, &ArticleViewerWebEngine::slotLoadFinished);
    //connect(page(), &QWebEnginePage::scrollRequested, mWebViewAccessKey, &MessageViewer::WebViewAccessKey::hideAccessKeys);
    connect(page(), &QWebEnginePage::linkHovered, this, &ArticleViewerWebEngine::slotLinkHovered);


    setContextMenuPolicy( Qt::DefaultContextMenu);
    mWebShortcutMenuManager = new KIO::KUriFilterSearchProviderActions(this);
    mShareServiceManager = new PimCommon::ShareServiceUrlManager(this);
    connect(mShareServiceManager, &PimCommon::ShareServiceUrlManager::serviceUrlSelected, this, &ArticleViewerWebEngine::slotServiceUrlSelected);
}

ArticleViewerWebEngine::~ArticleViewerWebEngine()
{

}

QVariantHash ArticleViewerWebEngine::introductionData() const
{
    QVariantHash data;
    data[QStringLiteral("icon")] = QStringLiteral("akregator");
    data[QStringLiteral("name")] = i18n("Akregator");
    data[QStringLiteral("subtitle")] = i18n("Akregator is a KDE news feed reader.");
    data[QStringLiteral("version")] = KAboutData::applicationData().version();
    return data;
}

void ArticleViewerWebEngine::showAboutPage()
{
    paintAboutScreen(QStringLiteral("introduction_akregator.html"), introductionData());
}

void ArticleViewerWebEngine::paintAboutScreen(const QString &templateName, const QVariantHash &data)
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

void ArticleViewerWebEngine::slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type)
{
    const QUrl url = mShareServiceManager->generateServiceUrl(mCurrentUrl.url(), QString(), type);
    mShareServiceManager->openUrl(url);
}

void ArticleViewerWebEngine::slotSaveLinkAs()
{
    QUrl tmp(mCurrentUrl);

    if (tmp.fileName().isEmpty()) {
        tmp = tmp.adjusted(QUrl::RemoveFilename);
        tmp.setPath(tmp.path() + QLatin1String("index.html"));
    }
    KParts::BrowserRun::simpleSave(tmp, tmp.fileName());
}

void ArticleViewerWebEngine::slotSaveImageOnDiskInFrame()
{
    slotSaveLinkAs();
}

void ArticleViewerWebEngine::slotCopyImageLocationInFrame()
{
    slotCopyLinkAddress();
}

void ArticleViewerWebEngine::slotCopyLinkAddress()
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

void ArticleViewerWebEngine::contextMenuEvent(QContextMenuEvent *e)
{
    displayContextMenu(e->pos());
}

void ArticleViewerWebEngine::slotShowContextMenu(const QPoint &pos)
{
    displayContextMenu(pos);
}

void ArticleViewerWebEngine::slotPrintPreview()
{
    PimCommon::KPimPrintPreviewDialog previewdlg(this);
    connect(&previewdlg, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter * printer) {
        render(printer);
    });
    previewdlg.exec();
}

void ArticleViewerWebEngine::slotPrint()
{
    QPrinter printer;

    QScopedPointer<QPrintDialog> dlg(new QPrintDialog(&printer));

    if (dlg && dlg->exec() == QDialog::Accepted) {
        render(&printer);
    }
}

void ArticleViewerWebEngine::slotCopy()
{
    triggerPageAction(QWebEnginePage::Copy);
}

void ArticleViewerWebEngine::slotLoadFinished()
{
    unsetCursor();
}

void ArticleViewerWebEngine::slotLoadStarted()
{
    //mWebViewAccessKey->hideAccessKeys();
    setCursor(Qt::WaitCursor);
}

void ArticleViewerWebEngine::displayContextMenu(const QPoint &pos)
{
    delete mContextMenuHitResult;
    mContextMenuHitResult = new MessageViewer::MailWebHitTestResult(page(), pos);
    mCurrentUrl = mContextMenuHitResult->linkUrl();
    if (URLHandlerWebEngineManager::instance()->handleContextMenuRequest(mCurrentUrl, mapToGlobal(pos), this)) {
        return;
    }

    QMenu popup(this);
    const bool contentSelected = mContextMenuHitResult->isContentSelected();
    if (!mCurrentUrl.isEmpty() && !contentSelected) {
        popup.addAction(createOpenLinkInNewTabAction(mCurrentUrl, this, SLOT(slotOpenLinkInForegroundTab()), &popup));
        popup.addAction(createOpenLinkInExternalBrowserAction(mCurrentUrl, this, SLOT(slotOpenLinkInBrowser()), &popup));
        popup.addSeparator();
        popup.addAction(mActionCollection->action(QStringLiteral("savelinkas")));
        popup.addAction(mActionCollection->action(QStringLiteral("copylinkaddress")));
        if (!mContextMenuHitResult->imageUrl().isEmpty()) {
            popup.addSeparator();
            popup.addAction(mActionCollection->action(QStringLiteral("copy_image_location")));
            popup.addAction(mActionCollection->action(QStringLiteral("saveas_imageurl")));
            if (adblockEnabled()) {
                popup.addSeparator();
                popup.addAction(mActionCollection->action(QStringLiteral("adblock_image")));
            }
#if 0
            //popup.addAction(mMsgView->shareImage());
#endif
            //TODO
        }
        popup.addSeparator();
        popup.addAction(mShareServiceManager->menu());
    } else {
        if (contentSelected) {
            popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_copy")));
            popup.addSeparator();
            mWebShortcutMenuManager->setSelectedText(page()->selectedText());
            mWebShortcutMenuManager->addWebShortcutsToMenu(&popup);
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
    delete mContextMenuHitResult;
    mContextMenuHitResult = Q_NULLPTR;
}

void ArticleViewerWebEngine::slotLinkHovered(const QString &link)
{
    QUrl url(linkOrImageUrlAt(QCursor::pos()));
    QString msg = URLHandlerWebEngineManager::instance()->statusBarMessage(url, this);
    if (msg.isEmpty()) {
        msg = link;
    }

    Q_EMIT showStatusBarMessage(msg);
}

void ArticleViewerWebEngine::keyReleaseEvent(QKeyEvent *e)
{
    if (Settings::self()->accessKeyEnabled()) {
        //mWebViewAccessKey->keyReleaseEvent(e);
    }
    QWebEngineView::keyReleaseEvent(e);
}

void ArticleViewerWebEngine::keyPressEvent(QKeyEvent *e)
{
    if (e && hasFocus()) {
        if (Settings::self()->accessKeyEnabled()) {
            //mWebViewAccessKey->keyPressEvent(e);
        }
    }
    QWebEngineView::keyPressEvent(e);
}

void ArticleViewerWebEngine::wheelEvent(QWheelEvent *e)
{
    if (Settings::self()->accessKeyEnabled()) {
        //mWebViewAccessKey->wheelEvent(e);
    }
    QWebEngineView::wheelEvent(e);
}

void ArticleViewerWebEngine::resizeEvent(QResizeEvent *e)
{
    if (Settings::self()->accessKeyEnabled()) {
        //mWebViewAccessKey->resizeEvent(e);
    }
    QWebEngineView::resizeEvent(e);
}

void ArticleViewerWebEngine::disableIntroduction()
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

void ArticleViewerWebEngine::setArticleAction(ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed)
{
    Q_EMIT articleAction(type, articleId, feed);
}

void ArticleViewerWebEngine::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton) {
        Q_EMIT showContextMenu(event->pos());
        mLastButtonClicked = RightButton;
    } else if (event->button() & Qt::MiddleButton) {
        mLastButtonClicked = MiddleButton;
    } else if (event->button() & Qt::LeftButton) {
        mLastButtonClicked = LeftButton;
    }
    QWebEngineView::mouseReleaseEvent(event);
}

void ArticleViewerWebEngine::slotLinkClicked(const QUrl &url)
{
    if (URLHandlerWebEngineManager::instance()->handleClick(url, this)) {
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

void ArticleViewerWebEngine::slotOpenLinkInForegroundTab()
{
    OpenUrlRequest req(mCurrentUrl);
    req.setOptions(OpenUrlRequest::NewTab);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerWebEngine::slotOpenLinkInBackgroundTab()
{
    OpenUrlRequest req(mCurrentUrl);
    req.setOptions(OpenUrlRequest::NewTab);
    req.setOpenInBackground(true);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerWebEngine::slotOpenLinkInBrowser()
{
    OpenUrlRequest req(mCurrentUrl);
    req.setOptions(OpenUrlRequest::ExternalBrowser);
    Q_EMIT signalOpenUrlRequest(req);
}

QUrl ArticleViewerWebEngine::linkOrImageUrlAt(const QPoint &global) const
{
    const QPoint local = page()->view()->mapFromGlobal(global);
    const MessageViewer::MailWebHitTestResult hit(page(), local);
    if (!hit.linkUrl().isEmpty()) {
        return hit.linkUrl();
    } else if (!hit.imageUrl().isEmpty()) {
        return hit.imageUrl();
    } else {
        return QUrl();
    }
}

bool ArticleViewerWebEngine::zoomTextOnlyInFrame() const
{
    return false;
    //return settings()->testAttribute(QWebEngineSettings::ZoomTextOnly);
}

void ArticleViewerWebEngine::slotZoomTextOnlyInFrame(bool textOnlyInFrame)
{
    //settings()->setAttribute(QWebEngineSettings::ZoomTextOnly, textOnlyInFrame);
}

void ArticleViewerWebEngine::slotBlockImage()
{
    if (mCurrentUrl.isEmpty()) {
        return;
    }
    //MessageViewer::AdBlockManager::self()->addCustomRule(mCurrentUrl.url(), true);
}

bool ArticleViewerWebEngine::adblockEnabled() const
{
    return false;
    //return MessageViewer::AdBlockManager::self()->isEnabled();
}

#if 0
ArticleViewerWebEngine::ArticleViewerWebEngine(KActionCollection *ac, QWidget *parent)
    : KWebView(parent, false),
      mActionCollection(ac),
      mLastButtonClicked(LeftButton)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setPage(new MessageViewer::WebPage(this));
    mWebViewAccessKey = new MessageViewer::WebViewAccessKey(this, this);
    mWebViewAccessKey->setActionCollection(mActionCollection);

    connect(this, &QWebView::loadStarted, this, &ArticleViewerWebEngine::slotLoadStarted);
    connect(this, &QWebView::loadFinished, this, &ArticleViewerWebEngine::slotLoadFinished);
    connect(page(), &QWebPage::scrollRequested, mWebViewAccessKey, &MessageViewer::WebViewAccessKey::hideAccessKeys);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(page(), &QWebPage::linkHovered, this, &ArticleViewerWebEngine::slotLinkHovered);
    connect(this, &ArticleViewerWebEngine::linkClicked, this, &ArticleViewerWebEngine::slotLinkClicked);
}

ArticleViewerWebEngine::~ArticleViewerWebEngine()
{
    disconnect(this, &QWebView::loadFinished, this, &ArticleViewerWebEngine::slotLoadFinished);
}

#endif
