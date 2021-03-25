/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QPoint>
#include <QString>
#include <QUrl>
namespace Akregator
{
class ArticleViewerWebEngine;
class URLHandlerWebEngine
{
public:
    virtual ~URLHandlerWebEngine()
    {
    }

    /**
     * Called when LMB-clicking on a link in the reader. Should start
     * processing equivalent to "opening" the link.
     *
     * @return true if the click was handled by this URLHandlerWebEngine,
     *         false otherwise.
     */
    virtual bool handleClick(const QUrl &url, ArticleViewerWebEngine *w) const = 0;

    /**
     * Called when RMB-clicking on a link in the reader. Should show
     * a context menu at the specified point with the specified
     * widget as parent.
     *
     * @return true if the right-click was handled by this
     * URLHandlerWebEngine, false otherwise.
     */
    virtual bool handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerWebEngine *w) const = 0;

    /**
     * Called when hovering over a link.
     *
     * @return a string to be shown in the status bar while hoverin
     * over this link.
     */
    virtual QString statusBarMessage(const QUrl &url, ArticleViewerWebEngine *w) const = 0;
};

class AkregatorConfigHandler : public URLHandlerWebEngine
{
public:
    AkregatorConfigHandler()
        : URLHandlerWebEngine()
    {
    }

    ~AkregatorConfigHandler() override
    {
    }

    Q_REQUIRED_RESULT bool handleClick(const QUrl &, ArticleViewerWebEngine *) const override;
    Q_REQUIRED_RESULT bool handleContextMenuRequest(const QUrl &, const QPoint &, ArticleViewerWebEngine *) const override;
    Q_REQUIRED_RESULT QString statusBarMessage(const QUrl &, ArticleViewerWebEngine *) const override;
};

class MailToURLHandlerWebEngine : public URLHandlerWebEngine
{
public:
    MailToURLHandlerWebEngine()
        : URLHandlerWebEngine()
    {
    }

    ~MailToURLHandlerWebEngine() override
    {
    }

    Q_REQUIRED_RESULT bool handleClick(const QUrl &, ArticleViewerWebEngine *) const override;
    Q_REQUIRED_RESULT bool handleContextMenuRequest(const QUrl &, const QPoint &, ArticleViewerWebEngine *) const override;
    Q_REQUIRED_RESULT QString statusBarMessage(const QUrl &, ArticleViewerWebEngine *) const override;
};

class ActionURLHandlerWebEngine : public URLHandlerWebEngine
{
public:
    ActionURLHandlerWebEngine()
        : URLHandlerWebEngine()
    {
    }

    ~ActionURLHandlerWebEngine() override
    {
    }

    Q_REQUIRED_RESULT bool handleClick(const QUrl &, ArticleViewerWebEngine *) const override;
    Q_REQUIRED_RESULT bool handleContextMenuRequest(const QUrl &, const QPoint &, ArticleViewerWebEngine *) const override;
    Q_REQUIRED_RESULT QString statusBarMessage(const QUrl &, ArticleViewerWebEngine *) const override;
};
}

