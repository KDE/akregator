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

#ifndef URLHANDLER_H
#define URLHANDLER_H

#include <QUrl>
#include <QString>
#include <QPoint>
namespace Akregator
{
class ArticleViewerNg;
class URLHandler
{
public:
    virtual ~URLHandler() {}
    /**
      * Called when LMB-clicking on a link in the reader. Should start
      * processing equivalent to "opening" the link.
      *
      * @return true if the click was handled by this URLHandler,
      *         false otherwise.
      */
    virtual bool handleClick(const QUrl &url, ArticleViewerNg *w) const = 0;

    /**
      * Called when RMB-clicking on a link in the reader. Should show
      * a context menu at the specified point with the specified
      * widget as parent.
      *
      * @return true if the right-click was handled by this
      * URLHandler, false otherwise.
      */
    virtual bool handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerNg *w) const = 0;

    /**
      * Called when hovering over a link.
      *
      * @return a string to be shown in the status bar while hoverin
      * over this link.
      */
    virtual QString statusBarMessage(const QUrl &url, ArticleViewerNg *w) const = 0;

    /**
     * Called when shift-clicking the link in the reader.
     * @return true if the click was handled by this URLHandler, false otherwise
     */
    virtual bool handleShiftClick(const QUrl &url, ArticleViewerNg *window) const
    {
        Q_UNUSED(url);
        Q_UNUSED(window);
        return false;
    }

    /**
     * @return should return true if this URLHandler will handle the drag
     */
    virtual bool willHandleDrag(const QUrl &url, ArticleViewerNg *window) const
    {
        Q_UNUSED(url);
        Q_UNUSED(window);
        return false;
    }

    /**
     * Called when starting a drag with the given URL.
     * If the drag is handled, you should create a drag object.
     * @return true if the click was handled by this URLHandler, false otherwise
     */
    virtual bool handleDrag(const QUrl &url, ArticleViewerNg *window) const
    {
        Q_UNUSED(url);
        Q_UNUSED(window);
        return false;
    }
};

class AkregatorConfigHandler : public URLHandler
{
public:
    AkregatorConfigHandler()
        : URLHandler() {}
    virtual ~AkregatorConfigHandler() {}
    bool handleClick(const QUrl &, ArticleViewerNg *) const Q_DECL_OVERRIDE;
    bool handleContextMenuRequest(const QUrl &, const QPoint &, ArticleViewerNg *) const Q_DECL_OVERRIDE;
    QString statusBarMessage(const QUrl &, ArticleViewerNg *) const Q_DECL_OVERRIDE;
};

class MailToURLHandler : public URLHandler
{
public:
    MailToURLHandler() : URLHandler() {}
    virtual ~MailToURLHandler() {}

    bool handleClick(const QUrl &, ArticleViewerNg *) const Q_DECL_OVERRIDE;
    bool handleContextMenuRequest(const QUrl &, const QPoint &, ArticleViewerNg *) const Q_DECL_OVERRIDE
    {
        return false;
    }
    QString statusBarMessage(const QUrl &, ArticleViewerNg *) const Q_DECL_OVERRIDE;
};

class ActionURLHandler : public URLHandler
{
public:
    ActionURLHandler() : URLHandler() {}
    virtual ~ActionURLHandler() {}

    bool handleClick(const QUrl &, ArticleViewerNg *) const Q_DECL_OVERRIDE;
    bool handleContextMenuRequest(const QUrl &, const QPoint &, ArticleViewerNg *) const Q_DECL_OVERRIDE
    {
        return false;
    }
    QString statusBarMessage(const QUrl &, ArticleViewerNg *) const Q_DECL_OVERRIDE;
};

}

#endif // URLHANDLER_H
