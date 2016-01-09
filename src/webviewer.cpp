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

#include "webviewer.h"
#include "actionmanager.h"
#include "actions.h"

#include <KActionCollection>

#include <QContextMenuEvent>
#include <QMenu>
#include <QWebHistory>

using namespace Akregator;

WebViewer::WebViewer(KActionCollection *ac, QWidget *parent)
    : ArticleViewerNg(ac, parent)
{
    settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebSettings::JavaEnabled, true);
    settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
    settings()->setAttribute(QWebSettings::AutoLoadImages, true);
}

WebViewer::~WebViewer()
{

}

void WebViewer::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu popup(this);
    QWebHistory *history = page()->history();
    if (history->canGoBack()) {
        popup.addAction(pageAction(QWebPage::Back));
    }

    if (history->canGoForward()) {
        popup.addAction(pageAction(QWebPage::Forward));
    }
    popup.addAction(pageAction(QWebPage::Reload));

    mContextMenuHitResult = page()->mainFrame()->hitTestContent(event->pos());
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
        popup.addAction(ActionManager::getInstance()->action(QStringLiteral("viewer_printpreview")));
    }
    popup.exec(mapToGlobal(event->pos()));
}
