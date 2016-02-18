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

#ifndef URLHANDLERWebEngineMANAGER_H
#define URLHANDLERWebEngineMANAGER_H

#include <QObject>
#include <QVector>
#include "akregator_export.h"

namespace Akregator
{
class URLHandlerWebEngine;
class ArticleViewerNg;
class AKREGATOR_EXPORT URLHandlerWebEngineManager : public QObject
{
    Q_OBJECT
public:
    ~URLHandlerWebEngineManager();

    static URLHandlerWebEngineManager *instance();
    void registerHandler(const URLHandlerWebEngine *handler);
    void unregisterHandler(const URLHandlerWebEngine *handler);

    bool handleClick(const QUrl &url, ArticleViewerNg *w = Q_NULLPTR) const;
    bool handleShiftClick(const QUrl &url, ArticleViewerNg *window = Q_NULLPTR) const;
    bool handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerNg *w = Q_NULLPTR) const;
    bool willHandleDrag(const QUrl &url, ArticleViewerNg *window = Q_NULLPTR) const;
    bool handleDrag(const QUrl &url, ArticleViewerNg *window = Q_NULLPTR) const;
    QString statusBarMessage(const QUrl &url, ArticleViewerNg *w = Q_NULLPTR) const;

private:
    explicit URLHandlerWebEngineManager(QObject *parent = Q_NULLPTR);
    static URLHandlerWebEngineManager *self;
    typedef QVector<const URLHandlerWebEngine *> HandlerList;
    HandlerList mHandlers;
};
}

#endif // URLHANDLERWebEngineMANAGER_H
