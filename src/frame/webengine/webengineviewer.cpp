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

#include "webengineviewer.h"
#include "articleviewer-ng/webengine/articleviewerwebenginepage.h"

#include "actionmanager.h"
#include "actions.h"
#include "webengine/urlhandlerwebenginemanager.h"

#include <KPIMTextEdit/TextToSpeech>
#include <MessageViewer/AdBlockBlockableItemsDialog>

#include <KIO/KUriFilterSearchProviderActions>
#include <KActionCollection>
#include <KActionMenu>
#include <KLocalizedString>
#include <QWebEngineSettings>
#include <QContextMenuEvent>
#include <QMenu>
#include <QWebEngineHistory>
#include <MessageViewer/WebHitTest>
#include <MessageViewer/WebHitTestResult>
#include <MessageViewer/NetworkAccessManagerWebEngine>

using namespace Akregator;

WebEngineViewer::WebEngineViewer(KActionCollection *ac, QWidget *parent)
    : ArticleViewerWebEngine(ac, parent)
{
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    settings()->setAttribute(QWebEngineSettings::AutoLoadImages, true);
}

WebEngineViewer::~WebEngineViewer()
{

}

void WebEngineViewer::contextMenuEvent(QContextMenuEvent *e)
{
    displayContextMenu(e->pos());
}

void WebEngineViewer::slotWebHitFinished(const MessageViewer::WebHitTestResult &result)
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

    const bool contentSelected = !selectedText().isEmpty();
    if (!contentSelected) {
        if (!mCurrentUrl.isEmpty()) {
            popup.addAction(createOpenLinkInNewTabAction(mCurrentUrl, this, SLOT(slotOpenLinkInForegroundTab()), &popup));
            popup.addAction(createOpenLinkInExternalBrowserAction(mCurrentUrl, this, SLOT(slotOpenLinkInBrowser()), &popup));
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_print")));
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_printpreview")));
#endif
    popup.addActions(mNetworkAccessManager->interceptorUrlActions(result));
    popup.addSeparator();
    popup.addAction(ActionManager::getInstance()->action(QStringLiteral("find_in_messages")));
    if (KPIMTextEdit::TextToSpeech::self()->isReady()) {
        popup.addSeparator();
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("speak_text")));
    }
    popup.exec(mapToGlobal(result.pos()));
}

void WebEngineViewer::displayContextMenu(const QPoint &pos)
{
    MessageViewer::WebHitTest *webHit = mPageEngine->hitTestContent(pos);
    connect(webHit, &MessageViewer::WebHitTest::finished, this, &WebEngineViewer::slotWebHitFinished);
}
