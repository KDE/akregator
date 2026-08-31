/*
   SPDX-FileCopyrightText: 2015-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "articleviewerwebengine.h"
#include "actionmanager.h"
#include "actions/actions.h"
#include "akregator_debug.h"
#include "akregatorconfig.h"
#include "articleviewerwebenginepage.h"
#include "config-akregator.h"
#include "webengine/urlhandlerwebenginemanager.h"
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
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QIcon>
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

using namespace Qt::Literals::StringLiterals;

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
    mNetworkAccessManager->addInterceptor(mBlockExternalReference);
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

void ArticleViewerWebEngine::printPreviewPage(QPrinter *printer)
{
    print(printer);
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
    return {
        {u"icon"_s, u"akregator"_s},
        {u"name"_s, i18n("Akregator")},
        {u"subtitle"_s, i18n("Akregator is a KDE news feed reader.")},
        {u"version"_s, KAboutData::applicationData().version()},
    };
}

void ArticleViewerWebEngine::showAboutPage()
{
    paintAboutScreen(u":/about/introduction_akregator.html"_s, introductionData());
}

void ArticleViewerWebEngine::paintAboutScreen(const QString &templateName, const QVariantHash &data)
{
    GrantleeTheme::ThemeManager manager(u"splashPage"_s, u"splash.theme"_s, nullptr, u"messageviewer/about/"_s);
    GrantleeTheme::Theme theme = manager.theme(u"default"_s);
    if (theme.isValid()) {
        setHtml(theme.render(templateName, data, QByteArrayLiteral("akregator")), QUrl::fromLocalFile(theme.absolutePath() + u'/'));
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

void ArticleViewerWebEngine::saveUrl(const QUrl &sourceUrl)
{
    QUrl url(sourceUrl);
    if (url.isEmpty()) {
        return;
    }
    if (url.fileName().isEmpty()) {
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + "/index.html"_L1);
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
            job->addMetaData(u"MaxCacheSize"_s, u"0"_s); // Don't store in http cache.
            job->addMetaData(u"cache"_s, u"cache"_s); // Use entry from cache if available.
            KJobWidgets::setWindow(job, this);
            if (Settings::self()->disableSaveAsNotification()) {
                job->setFinishedNotificationHidden(true);
            }
            job->uiDelegate()->setAutoErrorHandlingEnabled(true);
        }
    });
}

void ArticleViewerWebEngine::slotMute(bool mute)
{
    page()->setAudioMuted(mute);
}

void ArticleViewerWebEngine::copyUrlToClipboard(const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }
    QClipboard *cb = QApplication::clipboard();
    cb->setText(url.toString(), QClipboard::Clipboard);
    // don't set url to selection as it's a no-no according to a fd.o spec
    // which spec? Nobody seems to care (tested Firefox (3.5.10) Konqueror,and KMail (4.2.3)), so I re-enable the following line unless someone gives
    // a good reason to remove it again (bug 183022) --Frank
    cb->setText(url.toString(), QClipboard::Selection);
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
            QAction *saveLinkAsAction = popup.addAction(i18n("&Save Link As…"));
            connect(saveLinkAsAction, &QAction::triggered, this, [this, url = mCurrentUrl]() {
                saveUrl(url);
            });
            QAction *copyLinkAddressAction = popup.addAction(QIcon::fromTheme(u"edit-copy"_s), i18n("Copy &Link Address"));
            connect(copyLinkAddressAction, &QAction::triggered, this, [this, url = mCurrentUrl]() {
                copyUrlToClipboard(url);
            });
        }
        const QUrl imageUrl = result.imageUrl();
        if (!imageUrl.isEmpty()) {
            popup.addSeparator();
            QAction *copyImageUrlAction = popup.addAction(QIcon::fromTheme(u"view-media-visualization"_s), i18nc("@action", "Copy Image URL"));
            connect(copyImageUrlAction, &QAction::triggered, this, [this, imageUrl]() {
                copyUrlToClipboard(imageUrl);
            });
            QAction *saveImageAction = popup.addAction(i18nc("@action", "Save Image…"));
            connect(saveImageAction, &QAction::triggered, this, [this, imageUrl]() {
                saveUrl(imageUrl);
            });
        }
        popup.addSeparator();
        popup.addActions(viewerPluginActionList(MessageViewer::ViewerPluginInterface::NeedUrl));
        popup.addSeparator();
        popup.addAction(mShareServiceManager->menu());
    } else {
        popup.addAction(ActionManager::getInstance()->action(u"viewer_copy"_s));
        popup.addSeparator();
        mWebShortcutMenuManager->setSelectedText(page()->selectedText());
        mWebShortcutMenuManager->addWebShortcutsToMenu(&popup);
        popup.addSeparator();
        popup.addActions(viewerPluginActionList(MessageViewer::ViewerPluginInterface::NeedSelection));
    }
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(u"viewer_print"_s));
    popup.addAction(ActionManager::getInstance()->action(u"viewer_printpreview"_s));
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(u"tab_mute"_s));
    popup.addAction(ActionManager::getInstance()->action(u"tab_unmute"_s));
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(u"find_in_messages"_s));
#if HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(u"speak_text"_s));
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
    KGuiItem yesButton(i18n("Disable"));
    KGuiItem noButton(i18n("Keep Enabled"));
    if (KMessageBox::questionTwoActions(this,
                                        i18n("Are you sure you want to disable this introduction page?"),
                                        i18nc("@title:window", "Disable Introduction Page"),
                                        yesButton,
                                        noButton)
        == KMessageBox::PrimaryAction) {
        Settings::self()->setDisableIntroduction(true);
        Settings::self()->save();
    }
}

void ArticleViewerWebEngine::setArticleAction(ArticleViewerWebEngine::ArticleAction type, const QString &articleId, const QString &feed)
{
    Q_EMIT articleAction(type, articleId, feed);
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
    if (KMessageBox::SecondaryAction
        == KMessageBox::questionTwoActions(this,
                                           i18n("This web site is a malware, do you want to continue to show it?"),
                                           i18nc("@title:window", "Malware"),
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
        KMessageBox::error(this, i18n("The network is broken."), i18nc("@title:window", "Check Phishing Url"));
        break;
    case WebEngineViewer::CheckPhishingUrlUtil::InvalidUrl:
        KMessageBox::error(this, i18n("The url %1 is not valid.", url.toString()), i18nc("@title:window", "Check Phishing Url"));
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
            req.setOptions(OpenUrlRequest::Options::ExternalBrowser);
            break;
        case Settings::EnumLMBBehaviour::OpenInBackground:
            req.setOpenInBackground(true);
            req.setOptions(OpenUrlRequest::Options::NewTab);
            break;
        default:
            break;
        }
    } else if (mLastButtonClicked == MiddleButton) {
        switch (Settings::mMBBehaviour()) {
        case Settings::EnumMMBBehaviour::OpenInExternalBrowser:
            req.setOptions(OpenUrlRequest::Options::ExternalBrowser);
            break;
        case Settings::EnumMMBBehaviour::OpenInBackground:
            req.setOpenInBackground(true);
            req.setOptions(OpenUrlRequest::Options::NewTab);
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
    req.setOptions(OpenUrlRequest::Options::NewTab);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerWebEngine::slotOpenLinkInBackgroundTab()
{
    OpenUrlRequest req(mCurrentUrl);
    req.setOptions(OpenUrlRequest::Options::NewTab);
    req.setOpenInBackground(true);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerWebEngine::slotOpenLinkInBrowser()
{
    OpenUrlRequest req(mCurrentUrl);
    req.setOptions(OpenUrlRequest::Options::ExternalBrowser);
    Q_EMIT signalOpenUrlRequest(req);
}

void ArticleViewerWebEngine::createViewerPluginToolManager(KActionCollection *ac, QWidget *parent)
{
    mViewerPluginToolManager = new MessageViewer::ViewerPluginToolManager(parent, this);
    mViewerPluginToolManager->setActionCollection(ac);
    mViewerPluginToolManager->setPluginName(u"akregator"_s);
    mViewerPluginToolManager->setPluginDirectory(u"akregator/viewerplugin"_s);
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

#include "moc_articleviewerwebengine.cpp"
