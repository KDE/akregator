/*
   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "urlhandlerwebenginemanager.h"
#include "urlhandlerwebengine.h"

#include <articleviewer-ng/webengine/articleviewerwebengine.h>

using std::remove;

using namespace Akregator;

URLHandlerWebEngineManager *URLHandlerWebEngineManager::self = nullptr;

URLHandlerWebEngineManager::URLHandlerWebEngineManager(QObject *parent)
    : QObject(parent)
{
    registerHandler(new AkregatorConfigHandler());
    registerHandler(new MailToURLHandlerWebEngine());
    registerHandler(new ActionURLHandlerWebEngine());
}

URLHandlerWebEngineManager::~URLHandlerWebEngineManager() = default;

URLHandlerWebEngineManager *URLHandlerWebEngineManager::instance()
{
    if (!self) {
        self = new URLHandlerWebEngineManager();
    }
    return self;
}

void URLHandlerWebEngineManager::registerHandler(const URLHandlerWebEngine *handler)
{
    if (!handler) {
        return;
    }
    unregisterHandler(handler); // don't produce duplicates
    mHandlers.push_back(handler);
}

void URLHandlerWebEngineManager::unregisterHandler(const URLHandlerWebEngine *handler)
{
    // don't delete them, only remove them from the list!
    mHandlers.erase(remove(mHandlers.begin(), mHandlers.end(), handler), mHandlers.end());
}

bool URLHandlerWebEngineManager::handleClick(const QUrl &url, ArticleViewerWebEngine *w) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it) {
        if ((*it)->handleClick(url, w)) {
            return true;
        }
    }
    return false;
}

bool URLHandlerWebEngineManager::handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerWebEngine *w) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it) {
        if ((*it)->handleContextMenuRequest(url, p, w)) {
            return true;
        }
    }
    return false;
}

QString URLHandlerWebEngineManager::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *w) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it) {
        const QString msg = (*it)->statusBarMessage(url, w);
        if (!msg.isEmpty()) {
            return msg;
        }
    }
    return {};
}
