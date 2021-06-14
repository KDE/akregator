/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akrwebengineviewer.h"
#include "articleviewer-ng/webengine/articleviewerwebenginepage.h"

#include "actionmanager.h"
#include "actions.h"
#include "akregatorconfig.h"
#include "webengine/urlhandlerwebenginemanager.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KIO/KUriFilterSearchProviderActions>
#include <KPIMTextEdit/TextToSpeech>
#include <QContextMenuEvent>
#include <QMenu>
#include <QWebEngineHistory>
#include <QWebEngineSettings>
#include <WebEngineViewer/BlockExternalResourcesUrlInterceptor>
#include <WebEngineViewer/InterceptorManager>
#include <WebEngineViewer/LoadExternalReferencesUrlInterceptor>
#include <WebEngineViewer/WebHitTest>
#include <WebEngineViewer/WebHitTestResult>

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
    // settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);

    settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
}

AkrWebEngineViewer::~AkrWebEngineViewer()
{
}

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
    popup.addAction(pageAction(QWebEnginePage::SavePage));
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("tab_mute")));
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("tab_unmute")));
    const QList<QAction *> interceptorUrlActions = mNetworkAccessManager->interceptorUrlActions(result);
    if (!interceptorUrlActions.isEmpty()) {
        popup.addSeparator();
        popup.addActions(interceptorUrlActions);
    }
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("find_in_messages")));
    if (KPIMTextEdit::TextToSpeech::self()->isReady()) {
        popup.addSeparator();
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("speak_text")));
    }
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
