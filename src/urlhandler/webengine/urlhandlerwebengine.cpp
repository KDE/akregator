/*
   SPDX-FileCopyrightText: 2016-2024 Laurent Montel <montel@kde.org>

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
    if (url.scheme() == QLatin1StringView("config")) {
        if (url.path() == QLatin1StringView("/disable_introduction")) {
            article->disableIntroduction();
            return true;
        }
    }
    return false;
}

bool AkregatorConfigHandler::handleContextMenuRequest(const QUrl &url, const QPoint &, ArticleViewerWebEngine *) const
{
    return url.scheme() == QLatin1StringView("config");
}

QString AkregatorConfigHandler::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *) const
{
    if (url.scheme() == QLatin1StringView("config")) {
        if (url.path() == QLatin1StringView("/disable_introduction")) {
            return i18n("Disable Introduction");
        }
    }
    return {};
}

QString MailToURLHandlerWebEngine::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *) const
{
    if (url.scheme() == QLatin1StringView("mailto")) {
        return KEmailAddress::decodeMailtoUrl(url);
    }
    return {};
}

bool MailToURLHandlerWebEngine::handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerWebEngine *) const
{
    if (url.scheme() == QLatin1StringView("mailto")) {
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
    if (url.scheme() == QLatin1StringView("mailto")) {
        QDesktopServices::openUrl(url);
        return true;
    }
    return false;
}

bool ActionURLHandlerWebEngine::handleContextMenuRequest(const QUrl &url, const QPoint &, ArticleViewerWebEngine *) const
{
    return url.scheme() == QLatin1StringView("akregatoraction");
}

QString ActionURLHandlerWebEngine::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *) const
{
    if (url.scheme() == QLatin1StringView("akregatoraction")) {
        const QString urlPath(url.path());
        if (urlPath == QLatin1StringView("delete")) {
            return i18n("Delete Article");
        } else if (urlPath == QLatin1StringView("markAsRead")) {
            return i18n("Mark Article as Read");
        } else if (urlPath == QLatin1StringView("markAsUnRead")) {
            return i18n("Mark Article as Unread");
        } else if (urlPath == QLatin1StringView("markAsImportant")) {
            return i18n("Change Important Flag");
        } else if (urlPath == QLatin1StringView("sendUrlArticle")) {
            return i18n("Send the URL of the article");
        } else if (urlPath == QLatin1StringView("sendFileArticle")) {
            return i18n("Send the Html Page of Article");
        } else if (urlPath == QLatin1StringView("openInExternalBrowser")) {
            return i18n("Open In External Browser");
        } else if (urlPath == QLatin1StringView("openInBackgroundTab")) {
            return i18n("Open In Background Tab");
        }
        return {};
    }
    return {};
}

bool ActionURLHandlerWebEngine::handleClick(const QUrl &url, ArticleViewerWebEngine *articleViewer) const
{
    if (url.scheme() == QLatin1StringView("akregatoraction")) {
        const QString urlPath(url.path());
        if (url.hasQuery()) {
            const QUrlQuery urlQuery(url);
            const QString articleId = urlQuery.queryItemValue(QStringLiteral("id"));
            const QString feed = urlQuery.queryItemValue(QStringLiteral("feed"));
            if (!articleId.isEmpty()) {
                if (urlPath == QLatin1StringView("delete")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::DeleteAction, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1StringView("markAsRead")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::MarkAsRead, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1StringView("markAsUnRead")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::MarkAsUnRead, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1StringView("markAsImportant")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::MarkAsImportant, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1StringView("sendUrlArticle")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::SendUrlArticle, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1StringView("sendFileArticle")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::SendFileArticle, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1StringView("openInExternalBrowser")) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::OpenInExternalBrowser, articleId, feed);
                    return true;
                } else if (urlPath == QLatin1StringView("openInBackgroundTab")) {
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
