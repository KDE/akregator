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

#include "urlhandlerwebenginemanager.h"
#include "urlhandlerwebengine.h"
using std::remove;

using namespace Akregator;

URLHandlerWebEngineManager *URLHandlerWebEngineManager::self = 0;

URLHandlerWebEngineManager::URLHandlerWebEngineManager(QObject *parent)
    : QObject(parent)
{
    registerHandler(new AkregatorConfigHandler());
    registerHandler(new MailToURLHandlerWebEngine());
    registerHandler(new ActionURLHandlerWebEngine());
}

URLHandlerWebEngineManager::~URLHandlerWebEngineManager()
{

}

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
    unregisterHandler(handler);   // don't produce duplicates
    mHandlers.push_back(handler);
}

void URLHandlerWebEngineManager::unregisterHandler(const URLHandlerWebEngine *handler)
{
    // don't delete them, only remove them from the list!
    mHandlers.erase(remove(mHandlers.begin(), mHandlers.end(), handler), mHandlers.end());
}

bool URLHandlerWebEngineManager::handleClick(const QUrl &url, ArticleViewerNg *w) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->handleClick(url, w)) {
            return true;
        }
    return false;
}

bool URLHandlerWebEngineManager::handleShiftClick(const QUrl &url, ArticleViewerNg *window) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->handleShiftClick(url, window)) {
            return true;
        }
    return false;
}

bool URLHandlerWebEngineManager::handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerNg *w) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->handleContextMenuRequest(url, p, w)) {
            return true;
        }
    return false;
}

bool URLHandlerWebEngineManager::willHandleDrag(const QUrl &url, ArticleViewerNg *window) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->willHandleDrag(url, window)) {
            return true;
        }
    return false;
}

bool URLHandlerWebEngineManager::handleDrag(const QUrl &url, ArticleViewerNg *window) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->handleDrag(url, window)) {
            return true;
        }
    return false;
}

QString URLHandlerWebEngineManager::statusBarMessage(const QUrl &url, ArticleViewerNg *w) const
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
