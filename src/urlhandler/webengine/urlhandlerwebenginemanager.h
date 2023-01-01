/*
   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregator_export.h"
#include <QObject>
#include <QVector>

namespace Akregator
{
class URLHandlerWebEngine;
class ArticleViewerWebEngine;
class AKREGATOR_EXPORT URLHandlerWebEngineManager : public QObject
{
    Q_OBJECT
public:
    ~URLHandlerWebEngineManager() override;

    static URLHandlerWebEngineManager *instance();
    void registerHandler(const URLHandlerWebEngine *handler);
    void unregisterHandler(const URLHandlerWebEngine *handler);

    Q_REQUIRED_RESULT bool handleClick(const QUrl &url, ArticleViewerWebEngine *w = nullptr) const;
    Q_REQUIRED_RESULT bool handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerWebEngine *w = nullptr) const;
    Q_REQUIRED_RESULT QString statusBarMessage(const QUrl &url, ArticleViewerWebEngine *w = nullptr) const;

private:
    explicit URLHandlerWebEngineManager(QObject *parent = nullptr);
    static URLHandlerWebEngineManager *self;
    using HandlerList = QVector<const URLHandlerWebEngine *>;
    HandlerList mHandlers;
};
}
