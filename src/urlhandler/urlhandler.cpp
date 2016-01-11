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

#include "urlhandler.h"
#include "akregator_debug.h"

#include <articleviewer-ng/articleviewerng.h>
#include <KLocalizedString>
#include <QDesktopServices>
#include <KEmailAddress>
using namespace Akregator;

bool AkregatorConfigHandler::handleClick(const QUrl &url, ArticleViewerNg *article) const
{
    if (url.scheme() == QLatin1String("config")) {
        if (url.path() == QLatin1String("/disable_introduction")) {
            article->disableIntroduction();
            return true;
        }
    }
    return false;
}

bool AkregatorConfigHandler::handleContextMenuRequest(const QUrl &url, const QPoint &, ArticleViewerNg *) const
{
    return (url.scheme() == QLatin1String("config"));
}


QString AkregatorConfigHandler::statusBarMessage(const QUrl &url, ArticleViewerNg *) const
{
    if (url.scheme() == QLatin1String("config")) {
        if (url.path() == QLatin1String("/disable_introduction")) {
            return i18n("Disable Introduction");
        }
    }
    return {};
}

QString MailToURLHandler::statusBarMessage(const QUrl &url, ArticleViewerNg *) const
{
    if (url.scheme() == QLatin1String("mailto")) {
        return KEmailAddress::decodeMailtoUrl(url);
    }
    return QString();
}

bool MailToURLHandler::handleClick(const QUrl &url, ArticleViewerNg *) const
{
    if (url.scheme() == QLatin1String("mailto")) {
        QDesktopServices::openUrl(url);
        return true;
    }
    return false;
}


QString ActionURLHandler::statusBarMessage(const QUrl &url, ArticleViewerNg *) const
{
    if (url.scheme() == QLatin1String("akregatoraction")) {
        const QString urlPath(url.path());
        if (urlPath == QLatin1String("delete")) {
            return i18n("Delete Article");
        } else if (urlPath == QLatin1String("markAsRead")) {
            return i18n("Mark Article as Read");
        } else if (urlPath == QLatin1String("markAsUnRead")) {
            return i18n("Mark Article as Unread");
        } else if (urlPath == QLatin1String("markAsImportant")) {
            return i18n("Mark Article as Important");
        } else if (urlPath == QLatin1String("sendUrlArticle")) {
            return i18n("Send Url Article");
        } else if (urlPath == QLatin1String("openInExternalBrowser")) {
            return i18n("Open In External Browser");
        } else if (urlPath == QLatin1String("share")) {
            return i18n("Share");
        }
        return {};
    }
    return {};
}

bool ActionURLHandler::handleClick(const QUrl &url, ArticleViewerNg *articleViewer) const
{
    if (url.scheme() == QLatin1String("akregatoraction")) {
        const QString urlPath(url.path());
        const QString articleId = url.fragment();
        if (!articleId.isEmpty()) {
            if (urlPath == QLatin1String("delete")) {
                articleViewer->setArticleAction(ArticleViewerNg::DeleteAction, articleId);
                return true;
            } else if (urlPath == QLatin1String("markAsRead")) {
                articleViewer->setArticleAction(ArticleViewerNg::MarkAsRead, articleId);
                return true;
            } else if (urlPath == QLatin1String("markAsUnRead")) {
                articleViewer->setArticleAction(ArticleViewerNg::MarkAsUnRead, articleId);
                return true;
            } else if (urlPath == QLatin1String("markAsImportant")) {
                articleViewer->setArticleAction(ArticleViewerNg::MarkAsImportant, articleId);
                return true;
            } else if (urlPath == QLatin1String("sendUrlArticle")) {
                articleViewer->setArticleAction(ArticleViewerNg::SendUrlArticle, articleId);
                return true;
            } else if (urlPath == QLatin1String("openInExternalBrowser")) {
                articleViewer->setArticleAction(ArticleViewerNg::OpenInExternalBrowser, articleId);
                return true;
            } else if (urlPath == QLatin1String("share")) {
                articleViewer->setArticleAction(ArticleViewerNg::Share, articleId);
                return true;
            }
        } else {
            qCWarning(AKREGATOR_LOG) << "Undefined article id";
            return true;
        }
    }
    return false;
}
