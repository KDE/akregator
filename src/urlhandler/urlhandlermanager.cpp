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

#include "urlhandlermanager.h"
#include "urlhandler.h"
using std::remove;

using namespace Akregator;

URLHandlerManager *URLHandlerManager::self = 0;

URLHandlerManager::URLHandlerManager(QObject *parent)
    : QObject(parent)
{
    registerHandler(new AkregatorConfigHandler());
    registerHandler(new MailToURLHandler());
    registerHandler(new ActionURLHandler());
}

URLHandlerManager::~URLHandlerManager()
{

}

URLHandlerManager *URLHandlerManager::instance()
{
    if (!self) {
        self = new URLHandlerManager();
    }
    return self;
}

void URLHandlerManager::registerHandler(const URLHandler *handler)
{
    if (!handler) {
        return;
    }
    unregisterHandler(handler);   // don't produce duplicates
    mHandlers.push_back(handler);
}

void URLHandlerManager::unregisterHandler(const URLHandler *handler)
{
    // don't delete them, only remove them from the list!
    mHandlers.erase(remove(mHandlers.begin(), mHandlers.end(), handler), mHandlers.end());
}

bool URLHandlerManager::handleClick(const QUrl &url, ArticleViewerNg *w) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->handleClick(url, w)) {
            return true;
        }
    return false;
}

bool URLHandlerManager::handleShiftClick(const QUrl &url, ArticleViewerNg *window) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->handleShiftClick(url, window)) {
            return true;
        }
    return false;
}

bool URLHandlerManager::handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerNg *w) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->handleContextMenuRequest(url, p, w)) {
            return true;
        }
    return false;
}

bool URLHandlerManager::willHandleDrag(const QUrl &url, ArticleViewerNg *window) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->willHandleDrag(url, window)) {
            return true;
        }
    return false;
}

bool URLHandlerManager::handleDrag(const QUrl &url, ArticleViewerNg *window) const
{
    HandlerList::const_iterator end(mHandlers.constEnd());
    for (HandlerList::const_iterator it = mHandlers.constBegin(); it != end; ++it)
        if ((*it)->handleDrag(url, window)) {
            return true;
        }
    return false;
}

QString URLHandlerManager::statusBarMessage(const QUrl &url, ArticleViewerNg *w) const
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
