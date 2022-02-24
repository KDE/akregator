/*
   SPDX-FileCopyrightText: 2015-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "articleviewerwebengine.h"
#include "actionmanager.h"
#include "actions/actions.h"
#include "akregator_debug.h"
#include "akregatorconfig.h"
#include "articleviewerwebenginepage.h"
#include "urlhandler/webengine/urlhandlerwebengine.h"
#include "webengine/urlhandlerwebenginemanager.h"
#include <kpimtextedit/kpimtextedit-texttospeech.h>
#if KPIMTEXTEDIT_TEXT_TO_SPEECH
#include <KPIMTextEdit/TextToSpeech>
#endif
#include <WebEngineViewer/InterceptorManager>
#include <WebEngineViewer/WebEngineAccessKey>
#include <WebEngineViewer/WebEngineManageScript>
#include <WebEngineViewer/ZoomActionMenu>

#include <GrantleeTheme/GrantleeThemeManager>
#include <KAboutData>
#include <KActionCollection>
#include <KGuiItem>
#include <KIO/FileCopyJob>
#include <KJobUiDelegate>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <MessageViewer/ViewerPluginToolManager>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPrinter>
#include <QWebEngineProfile>
#include <QWebEngineUrlRequestInterceptor>

#include <WebEngineViewer/BlockExternalResourcesUrlInterceptor>
#include <WebEngineViewer/BlockTrackingUrlInterceptor>
#include <WebEngineViewer/LoadExternalReferencesUrlInterceptor>
#include <WebEngineViewer/WebEngineScript>
#include <WebEngineViewer/WebHitTest>
#include <WebEngineViewer/WebHitTestResult>

#include <WebEngineViewer/LocalDataBaseManager>

#include <KIO/KUriFilterSearchProviderActions>

using namespace Akregator;

class AkregatorRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    explicit AkregatorRequestInterceptor(QObject *parent = nullptr)
        : QWebEngineUrlRequestInterceptor(parent)
    {
    }

    void interceptRequest(QWebEngineUrlRequestInfo &info) override
    {
        Q_UNUSED(info)
    }
};
#define HAVE_BLOCK_SUPPORT 1
ArticleViewerWebEngine::ArticleViewerWebEngine(KActionCollection *ac, QWidget *parent)
    : WebEngineViewer::WebEngineView(parent)
    , mActionCollection(ac)
{
#ifndef HAVE_BLOCK_SUPPORT
    mNetworkAccessManager = new WebEngineViewer::InterceptorManager(this, ac, this);
#endif
    mPageEngine = new ArticleViewerWebEnginePage(this);
    QWebEngineProfile *profile = mPageEngine->profile();
    profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
#ifndef HAVE_BLOCK_SUPPORT
    // Needed to workaround crash in webengine, see https://bugreports.qt.io/browse/QTBUG-72260
    auto webEngineUrlInterceptor = new AkregatorRequestInterceptor();
    connect(profile, &QObject::destroyed, webEngineUrlInterceptor, &AkregatorRequestInterceptor::deleteLater);
    profile->setUrlRequestInterceptor(webEngineUrlInterceptor);
#endif
    setPage(mPageEngine);

#ifdef HAVE_BLOCK_SUPPORT
    mNetworkAccessManager = new WebEngineViewer::InterceptorManager(this, ac, this);
    mExternalReference = new WebEngineViewer::LoadExternalReferencesUrlInterceptor(this);
    mExternalReference->setAllowExternalContent(Settings::self()->loadExternalReferences());
    // connect(externalReference, &MessageViewer::LoadExternalReferencesUrlInterceptor::urlBlocked, this, &MailWebEngineView::urlBlocked);
    mNetworkAccessManager->addInterceptor(mExternalReference);
    auto blockTracking = new WebEngineViewer::BlockTrackingUrlInterceptor(this);
    connect(blockTracking, &WebEngineViewer::BlockTrackingUrlInterceptor::trackingFound, this, &ArticleViewerWebEngine::trackingFound);
    mNetworkAccessManager->addInterceptor(blockTracking);
    mBlockExternalReference = new WebEngineViewer::BlockExternalResourcesUrlInterceptor(this);
    // TODO connect(mBlockExternalReference, &WebEngineViewer::BlockExternalResourcesUrlInterceptor::formSubmittedForbidden, this,
    // &MailWebEngineView::urlBlocked);
#endif
    connect(this, &ArticleViewerWebEngine::showContextMenu, this, &ArticleViewerWebEngine::slotShowContextMenu);

    setFocusPolicy(Qt::WheelFocus);
    connect(mPageEngine, &ArticleViewerWebEnginePage::urlClicked, this, &ArticleViewerWebEngine::slotLinkClicked);

    mWebEngineViewAccessKey = new WebEngineViewer::WebEngineAccessKey(this, this);
    mWebEngineViewAccessKey->setActionCollection(mActionCollection);
    connect(mWebEngineViewAccessKey, &WebEngineViewer::WebEngineAccessKey::openUrl, this, &ArticleViewerWebEngine::slotLinkClicked);

    connect(this, &ArticleViewerWebEngine::loadStarted, this, &ArticleViewerWebEngine::slotLoadStarted);
    connect(this, &ArticleViewerWebEngine::loadFinished, this, &ArticleViewerWebEngine::slotLoadFinished);
    connect(page(), &QWebEnginePage::linkHovered, this, &ArticleViewerWebEngine::slotLinkHovered);

    setContextMenuPolicy(Qt::DefaultContextMenu);
    mWebShortcutMenuManager = new KIO::KUriFilterSearchProviderActions(this);
    mShareServiceManager = new PimCommon::ShareServiceUrlManager(this);
    connect(mShareServiceManager, &PimCommon::ShareServiceUrlManager::serviceUrlSelected, this, &ArticleViewerWebEngine::slotServiceUrlSelected);
    connect(page(), &QWebEnginePage::audioMutedChanged, this, &ArticleViewerWebEngine::slotWebPageMutedOrAudibleChanged);
    connect(page(), &QWebEnginePage::recentlyAudibleChanged, this, &ArticleViewerWebEngine::slotWebPageMutedOrAudibleChanged);

    connect(phishingDatabase(), &WebEngineViewer::LocalDataBaseManager::checkUrlFinished, this, &ArticleViewerWebEngine::slotCheckedUrlFinished);
}

ArticleViewerWebEngine::~ArticleViewerWebEngine() = default;

void ArticleViewerWebEngine::execPrintPreviewPage(QPrinter *printer, int timeout)
{
    if (!mPageEngine->execPrintPreviewPage(printer, timeout)) {
        qCWarning(AKREGATOR_LOG) << "Impossible to print page";
    }
}

void ArticleViewerWebEngine::updateSecurity()
{
    mExternalReference->setAllowExternalContent(Settings::self()->loadExternalReferences());
}

void ArticleViewerWebEngine::slotWebPageMutedOrAudibleChanged()
{
    Q_EMIT webPageMutedOrAudibleChanged(page()->isAudioMuted(), page()->recentlyAudible());
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
    paintAboutScreen(QStringLiteral(":/about/introduction_akregator.html"), introductionData());
}

void ArticleViewerWebEngine::paintAboutScreen(const QString &templateName, const QVariantHash &data)
{
    GrantleeTheme::ThemeManager manager(QStringLiteral("splashPage"), QStringLiteral("splash.theme"), nullptr, QStringLiteral("messageviewer/about/"));
    GrantleeTheme::Theme theme = manager.theme(QStringLiteral("default"));
    if (theme.isValid()) {
        setHtml(theme.render(templateName, data, QByteArrayLiteral("akregator")), QUrl::fromLocalFile(theme.absolutePath() + QLatin1Char('/')));
    } else {
        qCDebug(AKREGATOR_LOG) << "Theme error: failed to find splash theme";
    }
}

void ArticleViewerWebEngine::slotServiceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type)
{
    if (mCurrentUrl.isEmpty()) {
        return;
    }
    const QUrl url = mShareServiceManager->generateServiceUrl(mCurrentUrl.url(), QString(), type);
    mShareServiceManager->openUrl(url);
}

void ArticleViewerWebEngine::slotSaveLinkAs()
{
    QUrl url(mCurrentUrl);
    if (url.fileName().isEmpty()) {
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1String("/index.html"));
    }

    auto dlg = new QFileDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setAcceptMode(QFileDialog::AcceptSave);
    dlg->setWindowTitle(i18nc("@title:window", "Save As"));
    dlg->setOption(QFileDialog::DontConfirmOverwrite, false);
    dlg->selectFile(url.fileName());
    dlg->show();
    connect(dlg, &QFileDialog::urlSelected, this, [this, url](const QUrl &destURL) {
        if (destURL.isValid()) {
            KIO::FileCopyJob *job = KIO::file_copy(url, destURL, -1, KIO::Overwrite);
            job->addMetaData(QStringLiteral("MaxCacheSize"), QStringLiteral("0")); // Don't store in http cache.
            job->addMetaData(QStringLiteral("cache"), QStringLiteral("cache")); // Use entry from cache if available.
            KJobWidgets::setWindow(job, this);
            job->uiDelegate()->setAutoErrorHandlingEnabled(true);
        }
    });
}

void ArticleViewerWebEngine::slotSaveImageOnDiskInFrame()
{
    slotSaveLinkAs();
}

void ArticleViewerWebEngine::slotCopyImageLocationInFrame()
{
    slotCopyLinkAddress();
}

void ArticleViewerWebEngine::slotMute(bool mute)
{
    page()->setAudioMuted(mute);
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

void ArticleViewerWebEngine::slotCopy()
{
    triggerPageAction(QWebEnginePage::Copy);
}

void ArticleViewerWebEngine::slotLoadFinished()
{
    restoreCurrentPosition();
    unsetCursor();
}

void ArticleViewerWebEngine::slotLoadStarted()
{
    mWebEngineViewAccessKey->hideAccessKeys();
    setCursor(Qt::WaitCursor);
}

void ArticleViewerWebEngine::slotWebHitFinished(const WebEngineViewer::WebHitTestResult &result)
{
    mCurrentUrl = result.linkUrl();
    if (URLHandlerWebEngineManager::instance()->handleContextMenuRequest(mCurrentUrl, mapToGlobal(result.pos()), this)) {
        return;
    }

    QMenu popup(this);
    const bool noContentSelected = selectedText().isEmpty();
    if (noContentSelected) {
        if (!mCurrentUrl.isEmpty()) {
            {
                QAction *act = createOpenLinkInNewTabAction(mCurrentUrl, &popup);
                connect(act, &QAction::triggered, this, &ArticleViewerWebEngine::slotOpenLinkInBackgroundTab);
                popup.addAction(act);
            }
            {
                QAction *act = createOpenLinkInExternalBrowserAction(mCurrentUrl, &popup);
                connect(act, &QAction::triggered, this, &ArticleViewerWebEngine::slotOpenLinkInBrowser);
                popup.addAction(act);
            }
            popup.addSeparator();
            popup.addAction(mActionCollection->action(QStringLiteral("savelinkas")));
            popup.addAction(mActionCollection->action(QStringLiteral("copylinkaddress")));
        }
        if (!result.imageUrl().isEmpty()) {
            popup.addSeparator();
            popup.addAction(mActionCollection->action(QStringLiteral("copy_image_location")));
            popup.addAction(mActionCollection->action(QStringLiteral("saveas_imageurl")));
        }
        popup.addSeparator();
        popup.addActions(viewerPluginActionList(MessageViewer::ViewerPluginInterface::NeedUrl));
        popup.addSeparator();
        popup.addAction(mShareServiceManager->menu());
    } else {
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_copy")));
        popup.addSeparator();
        mWebShortcutMenuManager->setSelectedText(page()->selectedText());
        mWebShortcutMenuManager->addWebShortcutsToMenu(&popup);
        popup.addSeparator();
        popup.addActions(viewerPluginActionList(MessageViewer::ViewerPluginInterface::NeedSelection));
    }
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_print")));
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_printpreview")));
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("tab_mute")));
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("tab_unmute")));
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("find_in_messages")));
#if KPIMTEXTEDIT_TEXT_TO_SPEECH
    if (KPIMTextEdit::TextToSpeech::self()->isReady()) {
        popup.addSeparator();
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("speak_text")));
    }
#endif
    popup.exec(mapToGlobal(result.pos()));
}

void ArticleViewerWebEngine::displayContextMenu(const QPoint &pos)
{
    WebEngineViewer::WebHitTest *webHit = mPageEngine->hitTestContent(pos);
    connect(webHit, &WebEngineViewer::WebHitTest::finished, this, &ArticleViewerWebEngine::slotWebHitFinished);
}

void ArticleViewerWebEngine::slotLinkHovered(const QString &link)
{
    QString msg = URLHandlerWebEngineManager::instance()->statusBarMessage(QUrl(link), this);
    if (msg.isEmpty()) {
        msg = link;
    }

    Q_EMIT showStatusBarMessage(msg);
}

void ArticleViewerWebEngine::forwardKeyReleaseEvent(QKeyEvent *e)
{
    if (Settings::self()->accessKeyEnabled()) {
        mWebEngineViewAccessKey->keyReleaseEvent(e);
    }
}

void ArticleViewerWebEngine::forwardKeyPressEvent(QKeyEvent *e)
{
    if (e && hasFocus()) {
        if (Settings::self()->accessKeyEnabled()) {
            mWebEngineViewAccessKey->keyPressEvent(e);
        }
    }
}

void ArticleViewerWebEngine::forwardWheelEvent(QWheelEvent *e)
{
    if (Settings::self()->accessKeyEnabled()) {
        mWebEngineViewAccessKey->wheelEvent(e);
    }
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        const int numDegrees = e->angleDelta().y() / 8;
        const int numSteps = numDegrees / 15;
        const qreal factor = ActionManager::getInstance()->zoomActionMenu()->zoomFactor() + numSteps * 10;
        if (factor >= 10 && factor <= 300) {
            ActionManager::getInstance()->zoomActionMenu()->setZoomFactor(factor);
            ActionManager::getInstance()->zoomActionMenu()->setWebViewerZoomFactor(factor / 100.0);
        }
        e->accept();
    }
}

void ArticleViewerWebEngine::resizeEvent(QResizeEvent *e)
{
    if (Settings::self()->accessKeyEnabled()) {
        mWebEngineViewAccessKey->resizeEvent(e);
    }
    QWebEngineView::resizeEvent(e);
}

void ArticleViewerWebEngine::disableIntroduction()
{
    KGuiItem yesButton(KStandardGuiItem::yes());
    yesButton.setText(i18n("Disable"));
    KGuiItem noButton(KStandardGuiItem::no());
    noButton.setText(i18n("Keep Enabled"));
    if (KMessageBox::questionYesNo(this,
                                   i18n("Are you sure you want to disable this introduction page?"),
                                   i18n("Disable Introduction Page"),
                                   yesButton,
                                   noButton)
        == KMessageBox::Yes) {
        Settings::self()->setDisableIntroduction(true);
        Settings::self()->save();
    }
}

void ArticleViewerWebEngine::setArticleAction(ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed)
{
    Q_EMIT articleAction(type, articleId, feed);
}

void ArticleViewerWebEngine::restoreCurrentPosition()
{
    mPageEngine->runJavaScript(WebEngineViewer::WebEngineScript::scrollToRelativePosition(0), WebEngineViewer::WebEngineManageScript::scriptWordId());
}

void ArticleViewerWebEngine::forwardMouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton) {
        Q_EMIT showContextMenu(event->pos());
        mLastButtonClicked = RightButton;
    } else if (event->button() & Qt::MiddleButton) {
        mLastButtonClicked = MiddleButton;
    } else if (event->button() & Qt::LeftButton) {
        mLastButtonClicked = LeftButton;
    }
}

bool ArticleViewerWebEngine::urlIsAMalwareButContinue()
{
    if (KMessageBox::No
        == KMessageBox::warningYesNo(this,
                                     i18n("This web site is a malware, do you want to continue to show it?"),
                                     i18n("Malware"),
                                     KStandardGuiItem::cont(),
                                     KStandardGuiItem::cancel())) {
        return false;
    }
    return true;
}

void ArticleViewerWebEngine::slotCheckedUrlFinished(const QUrl &url, WebEngineViewer::CheckPhishingUrlUtil::UrlStatus status)
{
    switch (status) {
    case WebEngineViewer::CheckPhishingUrlUtil::BrokenNetwork:
        KMessageBox::error(this, i18n("The network is broken."), i18n("Check Phishing Url"));
        break;
    case WebEngineViewer::CheckPhishingUrlUtil::InvalidUrl:
        KMessageBox::error(this, i18n("The url %1 is not valid.", url.toString()), i18n("Check Phishing Url"));
        break;
    case WebEngineViewer::CheckPhishingUrlUtil::Ok:
        break;
    case WebEngineViewer::CheckPhishingUrlUtil::MalWare:
        if (!urlIsAMalwareButContinue()) {
            return;
        }
        break;
    case WebEngineViewer::CheckPhishingUrlUtil::Unknown:
        qCWarning(AKREGATOR_LOG) << "ArticleViewerWebEngine::slotCheckedUrlFinished unknown error ";
        break;
    }
    openSafeUrl(url);
}

void ArticleViewerWebEngine::slotLinkClicked(const QUrl &url)
{
    if (URLHandlerWebEngineManager::instance()->handleClick(url, this)) {
        return;
    }
    if (Settings::checkPhishingUrl()) {
        phishingDatabase()->checkUrl(url);
    } else {
        openSafeUrl(url);
    }
}

void ArticleViewerWebEngine::openSafeUrl(const QUrl &url)
{
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

void ArticleViewerWebEngine::createViewerPluginToolManager(KActionCollection *ac, QWidget *parent)
{
    mViewerPluginToolManager = new MessageViewer::ViewerPluginToolManager(parent, this);
    mViewerPluginToolManager->setActionCollection(ac);
    mViewerPluginToolManager->setPluginName(QStringLiteral("akregator"));
    mViewerPluginToolManager->setPluginDirectory(QStringLiteral("akregator/viewerplugin"));
    if (!mViewerPluginToolManager->initializePluginList()) {
        qCWarning(AKREGATOR_LOG) << " Impossible to initialize plugins";
    }
    mViewerPluginToolManager->createView();
    connect(mViewerPluginToolManager, &MessageViewer::ViewerPluginToolManager::activatePlugin, this, &ArticleViewerWebEngine::slotActivatePlugin);
}

QList<QAction *> ArticleViewerWebEngine::viewerPluginActionList(MessageViewer::ViewerPluginInterface::SpecificFeatureTypes features)
{
    if (mViewerPluginToolManager) {
        return mViewerPluginToolManager->viewerPluginActionList(features);
    }
    return {};
}

void ArticleViewerWebEngine::slotActivatePlugin(MessageViewer::ViewerPluginInterface *interface)
{
    const QString text = selectedText();
    if (!text.isEmpty()) {
        interface->setText(text);
    }
    interface->setUrl(mCurrentUrl);
    interface->execute();
}

#include "articleviewerwebengine.moc"
