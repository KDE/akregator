/*
   SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>

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

using namespace Qt::Literals::StringLiterals;
using namespace Akregator;

bool AkregatorConfigHandler::handleClick(const QUrl &url, ArticleViewerWebEngine *article) const
{
    if (url.scheme() == "config"_L1) {
        if (url.path() == "/disable_introduction"_L1) {
            article->disableIntroduction();
            return true;
        }
    }
    return false;
}

bool AkregatorConfigHandler::handleContextMenuRequest(const QUrl &url, const QPoint &, ArticleViewerWebEngine *) const
{
    return url.scheme() == "config"_L1;
}

QString AkregatorConfigHandler::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *) const
{
    if (url.scheme() == "config"_L1) {
        if (url.path() == "/disable_introduction"_L1) {
            return i18n("Disable Introduction");
        }
    }
    return {};
}

QString MailToURLHandlerWebEngine::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *) const
{
    if (url.scheme() == "mailto"_L1) {
        return KEmailAddress::decodeMailtoUrl(url);
    }
    return {};
}

bool MailToURLHandlerWebEngine::handleContextMenuRequest(const QUrl &url, const QPoint &p, ArticleViewerWebEngine *) const
{
    if (url.scheme() == "mailto"_L1) {
        auto menu = new QMenu();
        const QAction *copy = menu->addAction(QIcon::fromTheme(u"edit-copy"_s), i18n("&Copy Email Address"));

        const QAction *a = menu->exec(p);
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
    if (url.scheme() == "mailto"_L1) {
        QDesktopServices::openUrl(url);
        return true;
    }
    return false;
}

bool ActionURLHandlerWebEngine::handleContextMenuRequest(const QUrl &url, const QPoint &, ArticleViewerWebEngine *) const
{
    return url.scheme() == "akregatoraction"_L1;
}

QString ActionURLHandlerWebEngine::statusBarMessage(const QUrl &url, ArticleViewerWebEngine *) const
{
    if (url.scheme() == "akregatoraction"_L1) {
        const QString urlPath(url.path());
        if (urlPath == "delete"_L1) {
            return i18n("Delete Article");
        } else if (urlPath == "markAsRead"_L1) {
            return i18n("Mark Article as Read");
        } else if (urlPath == "markAsUnRead"_L1) {
            return i18n("Mark Article as Unread");
        } else if (urlPath == "markAsImportant"_L1) {
            return i18n("Change Important Flag");
        } else if (urlPath == "sendUrlArticle"_L1) {
            return i18n("Send the URL of the article");
        } else if (urlPath == "sendFileArticle"_L1) {
            return i18n("Send the Html Page of Article");
        } else if (urlPath == "openInExternalBrowser"_L1) {
            return i18n("Open In External Browser");
        } else if (urlPath == "openInBackgroundTab"_L1) {
            return i18n("Open In Background Tab");
        }
        return {};
    }
    return {};
}

bool ActionURLHandlerWebEngine::handleClick(const QUrl &url, ArticleViewerWebEngine *articleViewer) const
{
    if (url.scheme() == "akregatoraction"_L1) {
        const QString urlPath(url.path());
        if (url.hasQuery()) {
            const QUrlQuery urlQuery(url);
            const QString articleId = urlQuery.queryItemValue(u"id"_s);
            const QString feed = urlQuery.queryItemValue(u"feed"_s);
            if (!articleId.isEmpty()) {
                if (urlPath == "delete"_L1) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::DeleteAction, articleId, feed);
                    return true;
                } else if (urlPath == "markAsRead"_L1) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::MarkAsRead, articleId, feed);
                    return true;
                } else if (urlPath == "markAsUnRead"_L1) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::MarkAsUnRead, articleId, feed);
                    return true;
                } else if (urlPath == "markAsImportant"_L1) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::MarkAsImportant, articleId, feed);
                    return true;
                } else if (urlPath == "sendUrlArticle"_L1) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::SendUrlArticle, articleId, feed);
                    return true;
                } else if (urlPath == "sendFileArticle"_L1) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::SendFileArticle, articleId, feed);
                    return true;
                } else if (urlPath == "openInExternalBrowser"_L1) {
                    articleViewer->setArticleAction(ArticleViewerWebEngine::OpenInExternalBrowser, articleId, feed);
                    return true;
                } else if (urlPath == "openInBackgroundTab"_L1) {
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
