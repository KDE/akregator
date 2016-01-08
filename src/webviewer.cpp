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

    popup.exec(mapToGlobal(event->pos()));

#if 0
    //TODO

    const bool showReload = (flags & BrowserExtension::ShowReload) != 0;
    const bool showNavigationItems = (flags & BrowserExtension::ShowNavigationItems) != 0;
    const bool isLink = (flags & BrowserExtension:: IsLink) != 0;
    const bool isSelection = (flags & BrowserExtension::ShowTextSelectionItems) != 0;

    bool isFirst = true;

    QPointer<QMenu> popup(new QMenu());

    if (showNavigationItems) {
        popup->addAction(ActionManager::getInstance()->action(QStringLiteral("browser_back")));
        popup->addAction(ActionManager::getInstance()->action(QStringLiteral("browser_forward")));
        isFirst = false;
    }
    if (showReload) {
        popup->addAction(ActionManager::getInstance()->action(QStringLiteral("browser_reload")));
        isFirst = false;
    }

#define addSeparatorIfNotFirst() if ( !isFirst ) popup->addSeparator(); isFirst = false;

    if (isLink) {
        addSeparatorIfNotFirst();
        popup->addAction(createOpenLinkInNewTabAction(url, this, SLOT(slotOpenLinkInNewTab()), popup));
        popup->addAction(createOpenLinkInExternalBrowserAction(url, this, SLOT(slotOpenLinkInBrowser()), popup));
        addActionsToMenu(popup, actionGroups.value(QStringLiteral("linkactions")), ShowSeparatorIfNotEmpty);
    }

    if (isSelection) {
        addSeparatorIfNotFirst();
        addActionsToMenu(popup, actionGroups.value(QStringLiteral("editactions")), NoSeparator);
    }

    if (hasZoom()) {
        addSeparatorIfNotFirst();
        popup->addAction(ActionManager::getInstance()->action(QStringLiteral("inc_font_sizes")));
        popup->addAction(ActionManager::getInstance()->action(QStringLiteral("dec_font_sizes")));
    }

    addSeparatorIfNotFirst();
    addActionsToMenu(popup, actionGroups.value(QStringLiteral("part")), NoSeparator);

    popup->exec(global);
    delete popup;
#endif
}
