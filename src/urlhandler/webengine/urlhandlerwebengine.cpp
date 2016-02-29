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

#include "urlhandlerwebengine.h"
#include "akregator_debug.h"
#include "Libkdepim/BroadcastStatus"
#include "articleviewer-ng/webengine/articleviewerwebengine.h"
#include <KLocalizedString>
#include <QDesktopServices>
#include <QClipboard>
#include <QMenu>
#include <QApplication>
#include <KEmailAddress>
#include <OpenEmailAddressJob>
#include <QUrlQuery>
using namespace Akregator;

bool AkregatorConfigHandler::handleClick(const QUrl &url, ArticleViewerWebEngine *article) const
{
    if (url.scheme() == QLatin1String("config")) {
        if (url.path() == QLatin1String("/disable_introduction")) {
            article->disableIntroduction();
            return true;
        }
    }
    return false;
}

bool AkregatorConfigHandler::handleContextMenuRequest(const QUrl &url, const QPoint &, ArticleViewerWebEngine *) const
{
    return (url.scheme() == QLatin1String("config"));
}

QString AkregatorConfigHandler::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *) const
{
    if (url.scheme() == QLatin1String("config")) {
        if (url.path() == QLatin1String("/disable_introduction")) {
            return i18n("Disable Introduction");
        }
    }
    return {};
}

QString MailToURLHandlerWebEngine::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *) const
{
    if (url.scheme() == QLatin1String("mailto")) {
        return KEmailAddress::decodeMailtoUrl(url);
    }
    return QString();
}

void MailToURLHandlerWebEngine::runKAddressBook(const QUrl &url) const
{
    KPIM::OpenEmailAddressJob *job = new KPIM::OpenEmailAddressJob(url.path(), 0);
    job->start();
}

bool MailToURLHandlerWebEngine::handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerWebEngine *) const
{
    if (url.scheme() == QLatin1String("mailto")) {
        QMenu *menu = new QMenu();
        QAction *open = menu->addAction(QIcon::fromTheme(QStringLiteral("view-pim-contacts")), i18n("&Open in Address Book"));
        QAction *copy = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("&Copy Email Address"));

        QAction *a = menu->exec(p);
        if (a == copy) {
            const QString fullEmail = KEmailAddress::decodeMailtoUrl(url);
            if (!fullEmail.isEmpty()) {
                QClipboard *clip = QApplication::clipboard();
                clip->setText(fullEmail, QClipboard::Clipboard);
                clip->setText(fullEmail, QClipboard::Selection);
                KPIM::BroadcastStatus::instance()->setStatusMsg(i18n("Address copied to clipboard."));
            }
        } else if (a == open) {
            runKAddressBook(url);
        }
        delete menu;

        return true;
    }
    return false;
}

bool MailToURLHandlerWebEngine::handleClick(const QUrl &url, ArticleViewerWebEngine *) const
{
    if (url.scheme() == QLatin1String("mailto")) {
        QDesktopServices::openUrl(url);
        return true;
    }
    return false;
}

bool ActionURLHandlerWebEngine::handleContextMenuRequest(const QUrl &url, const QPoint &, ArticleViewerWebEngine *) const
{
    return (url.scheme() == QLatin1String("akregatoraction"));
}

QString ActionURLHandlerWebEngine::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *) const
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
            return i18n("Change Important Flag");
        } else if (urlPath == QLatin1String("sendUrlArticle")) {
            return i18n("Send the URL of the article");
        } else if (urlPath == QLatin1String("sendFileArticle")) {
            return i18n("Send the Html Page of Article");
        } else if (urlPath == QLatin1String("openInExternalBrowser")) {
            return i18n("Open In External Browser");
        } else if (urlPath == QLatin1String("share")) {
            return i18n("Share");
        } else if (urlPath == QLatin1String("openInBackgroundTab")) {
            return i18n("Open In Background Tab");
        }
        return {};
    }
    return {};
}

bool ActionURLHandlerWebEngine::handleClick(const QUrl &url, ArticleViewerWebEngine *articleViewer) const
{
    if (url.scheme() == QLatin1String("akregatoraction")) {
        const QString urlPath(url.path());
        if (url.hasQuery()) {
            const QUrlQuery urlQuery(url);
            const QString articleId = urlQuery.queryItemValue(QStringLiteral("id"));
            const QString feed = urlQuery.queryItemValue(QStringLiteral("feed"));
            if (!articleId.isEmpty()) {
                if (urlPath == QLatin1String("delete")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::DeleteAction, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1String("markAsRead")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::MarkAsRead, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1String("markAsUnRead")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::MarkAsUnRead, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1String("markAsImportant")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::MarkAsImportant, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1String("sendUrlArticle")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::SendUrlArticle, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1String("sendFileArticle")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::SendFileArticle, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1String("openInExternalBrowser")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::OpenInExternalBrowser, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1String("share")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::Share, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1String("openInBackgroundTab")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::OpenInBackgroundTab, articleId, feed);
                    return true;
                }
            }
        } else {
            qCWarning(AKREGATOR_LOG) << "Undefined article id";
            return true;
        }
    }
    return false;
}
