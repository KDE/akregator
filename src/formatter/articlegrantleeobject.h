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

#ifndef ARTICLEGRANTLEEOBJECT_H
#define ARTICLEGRANTLEEOBJECT_H

#include <QObject>
#include <article.h>
#include "articleformatter.h"
#include <QUrl>
namespace Akregator {
class ArticleGrantleeObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString strippedTitle READ strippedTitle)
    Q_PROPERTY(QString author READ author)
    Q_PROPERTY(QString content READ content)
    Q_PROPERTY(QString articleLinkUrl READ articleLinkUrl)
    Q_PROPERTY(QString articlePubDate READ articlePubDate)
    Q_PROPERTY(QString enclosure READ enclosure)
    Q_PROPERTY(QString articleCompleteStoryLink READ articleCompleteStoryLink)
    Q_PROPERTY(QString imageFeed READ imageFeed)
    Q_PROPERTY(QString deleteAction READ deleteAction)
    Q_PROPERTY(QString markAsReadAction READ markAsReadAction)
    Q_PROPERTY(QString markAsUnreadAction READ markAsUnreadAction)
    Q_PROPERTY(QString markAsImportantAction READ markAsImportantAction)
    Q_PROPERTY(QString sendUrlAction READ sendUrlAction)
    Q_PROPERTY(QString sendFileAction READ sendFileAction)
    Q_PROPERTY(QString openInExternalBrowser READ openInExternalBrowser)
    Q_PROPERTY(QString openInBackgroundTab READ openInBackgroundTab)
    Q_PROPERTY(Akregator::ArticleGrantleeObject::ArticleStatus articleStatus READ articleStatus)
    Q_PROPERTY(bool important READ important)

public:
    explicit ArticleGrantleeObject(const QUrl &imageDir, const Article &article, ArticleFormatter::IconOption icon, QObject *parent = nullptr);
    ~ArticleGrantleeObject();

    enum ArticleStatus {
        Unread = 0,
        Read,
        New
    };
    Q_ENUMS(ArticleStatus)

    Akregator::ArticleGrantleeObject::ArticleStatus articleStatus() const;

    QString strippedTitle() const;
    QString author() const;
    QString content() const;
    QString articleLinkUrl() const;
    QString articlePubDate() const;
    QString enclosure() const;
    QString articleCompleteStoryLink() const;
    QString imageFeed() const;

    //Action
    QString deleteAction() const;
    QString markAsReadAction() const;
    QString markAsUnreadAction() const;
    QString markAsImportantAction() const;
    QString sendUrlAction() const;
    QString sendFileAction() const;
    QString openInExternalBrowser() const;

    QString openInBackgroundTab() const;

    bool important() const;

private:
    QString createActionUrl(const QString &actionName) const;
    Article mArticle;
    ArticleFormatter::IconOption mArticleFormatOption;
    QUrl mImageDir;
    int mIconSize;
};
}
#endif // ARTICLEGRANTLEEOBJECT_H
