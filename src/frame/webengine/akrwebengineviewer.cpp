/*
   SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akrwebengineviewer.h"
#include "articleviewer-ng/webengine/articleviewerwebenginepage.h"
#include "config-akregator.h"

#include "actionmanager.h"
#include "actions.h"
#include "akregatorconfig.h"
#include "webengine/urlhandlerwebenginemanager.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KIO/KUriFilterSearchProviderActions>
#include <KLocalizedString>
#include <QAction>
#include <QContextMenuEvent>
#include <QIcon>
#include <QMenu>
#include <QWebEngineHistory>
#include <QWebEngineSettings>
#include <WebEngineViewer/BlockExternalResourcesUrlInterceptor>
#include <WebEngineViewer/InterceptorManager>
#include <WebEngineViewer/LoadExternalReferencesUrlInterceptor>
#include <WebEngineViewer/WebHitTest>
#include <WebEngineViewer/WebHitTestResult>

using namespace Qt::Literals::StringLiterals;

using namespace Akregator;

AkrWebEngineViewer::AkrWebEngineViewer(KActionCollection *ac, QWidget *parent)
    : ArticleViewerWebEngine(ac, parent)
{
    if (mExternalReference) {
        mExternalReference->setAllowExternalContent(true);
    }
    if (mBlockExternalReference) { // Don't block element
        mBlockExternalReference->setEnabled(false);
    }
    // TODO update settings when we change config
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, Settings::enableJavascript());
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    settings()->setAttribute(QWebEngineSettings::AutoLoadImages, true);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings()->setAttribute(QWebEngineSettings::XSSAuditingEnabled, false);
    settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    settings()->setAttribute(QWebEngineSettings::NavigateOnDropEnabled, false);
    // settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);

    settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
}

AkrWebEngineViewer::~AkrWebEngineViewer() = default;

void AkrWebEngineViewer::updateSecurity()
{
    // Nothing
}

void AkrWebEngineViewer::contextMenuEvent(QContextMenuEvent *e)
{
    displayContextMenu(e->pos());
}

void AkrWebEngineViewer::slotWebHitFinished(const WebEngineViewer::WebHitTestResult &result)
{
    mCurrentUrl = result.linkUrl();
    if (URLHandlerWebEngineManager::instance()->handleContextMenuRequest(mCurrentUrl, mapToGlobal(result.pos()), this)) {
        return;
    }
    QMenu popup(this);
    QWebEngineHistory *history = page()->history();
    bool needSeparator = false;
    if (history->canGoBack()) {
        popup.addAction(pageAction(QWebEnginePage::Back));
        needSeparator = true;
    }

    if (history->canGoForward()) {
        popup.addAction(pageAction(QWebEnginePage::Forward));
        needSeparator = true;
    }
    if (needSeparator) {
        popup.addSeparator();
    }
    popup.addAction(pageAction(QWebEnginePage::Stop));
    popup.addSeparator();
    popup.addAction(pageAction(QWebEnginePage::Reload));
    popup.addAction(mActionCollection->action(u"tab_copyurl"_s));
    popup.addSeparator();

    const bool noContentSelected = selectedText().isEmpty();
    if (noContentSelected) {
        if (!mCurrentUrl.isEmpty()) {
            {
                QAction *act = createOpenLinkInNewTabAction(mCurrentUrl, &popup);
                connect(act, &QAction::triggered, this, &AkrWebEngineViewer::slotOpenLinkInBackgroundTab);
                popup.addAction(act);
            }
            {
                QAction *act = createOpenLinkInExternalBrowserAction(mCurrentUrl, &popup);
                connect(act, &QAction::triggered, this, &AkrWebEngineViewer::slotOpenLinkInBrowser);
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
    popup.addAction(pageAction(QWebEnginePage::SavePage));
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(u"tab_mute"_s));
    popup.addAction(ActionManager::getInstance()->action(u"tab_unmute"_s));
    const QList<QAction *> interceptorUrlActions = mNetworkAccessManager->interceptorUrlActions(result);
    if (!interceptorUrlActions.isEmpty()) {
        popup.addSeparator();
        popup.addActions(interceptorUrlActions);
    }
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(u"find_in_messages"_s));
#if HAVE_KTEXTADDONS_TEXT_TO_SPEECH_SUPPORT
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(u"speak_text"_s));
#endif
    popup.exec(mapToGlobal(result.pos()));
}

void AkrWebEngineViewer::displayContextMenu(const QPoint &pos)
{
    WebEngineViewer::WebHitTest *webHit = mPageEngine->hitTestContent(pos);
    connect(webHit, &WebEngineViewer::WebHitTest::finished, this, &AkrWebEngineViewer::slotWebHitFinished);
}

QWebEngineView *AkrWebEngineViewer::createWindow(QWebEnginePage::WebWindowType type)
{
    Q_UNUSED(type)
    return this;
}

#include "moc_akrwebengineviewer.cpp"
