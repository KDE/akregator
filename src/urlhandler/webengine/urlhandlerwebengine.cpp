/*
   SPDX-FileCopyrightText: 2016-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "urlhandlerwebengine.h"
#include "akregator_debug.h"
#include "articleviewer-ng/webengine/articleviewerwebengine.h"
#include <KEmailAddress>
#include <KLocalizedString>
#include <PimCommon/BroadcastStatus>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QMenu>
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
    return url.scheme() == QLatin1String("config");
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
    return {};
}

bool MailToURLHandlerWebEngine::handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerWebEngine *) const
{
    if (url.scheme() == QLatin1String("mailto")) {
        auto menu = new QMenu();
        QAction *copy = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-copy")), i18n("&Copy Email Address"));

        QAction *a = menu->exec(p);
        if (a == copy) {
            const QString fullEmail = KEmailAddress::decodeMailtoUrl(url);
            if (!fullEmail.isEmpty()) {
                QClipboard *clip = QApplication::clipboard();
                clip->setText(fullEmail, QClipboard::Clipboard);
                clip->setText(fullEmail, QClipboard::Selection);
                PimCommon::BroadcastStatus::instance()->setStatusMsg(i18n("Address copied to clipboard."));
            }
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
    return url.scheme() == QLatin1String("akregatoraction");
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
