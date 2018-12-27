/*
   Copyright (C) 2016-2019 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef URLHANDLERWebEngine_H
#define URLHANDLERWebEngine_H

#include <QUrl>
#include <QString>
#include <QPoint>
namespace Akregator {
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

    bool handleClick(const QUrl &, ArticleViewerWebEngine *) const override;
    bool handleContextMenuRequest(const QUrl &, const QPoint &, ArticleViewerWebEngine *) const override;
    QString statusBarMessage(const QUrl &, ArticleViewerWebEngine *) const override;
};

class MailToURLHandlerWebEngine : public URLHandlerWebEngine
{
public:
    MailToURLHandlerWebEngine() : URLHandlerWebEngine()
    {
    }

    ~MailToURLHandlerWebEngine() override
    {
    }

    bool handleClick(const QUrl &, ArticleViewerWebEngine *) const override;
    bool handleContextMenuRequest(const QUrl &, const QPoint &, ArticleViewerWebEngine *) const override;
    QString statusBarMessage(const QUrl &, ArticleViewerWebEngine *) const override;
};

class ActionURLHandlerWebEngine : public URLHandlerWebEngine
{
public:
    ActionURLHandlerWebEngine() : URLHandlerWebEngine()
    {
    }

    ~ActionURLHandlerWebEngine() override
    {
    }

    bool handleClick(const QUrl &, ArticleViewerWebEngine *) const override;
    bool handleContextMenuRequest(const QUrl &, const QPoint &, ArticleViewerWebEngine *) const override;
    QString statusBarMessage(const QUrl &, ArticleViewerWebEngine *) const override;
};
}

#endif // URLHANDLERWebEngine_H
