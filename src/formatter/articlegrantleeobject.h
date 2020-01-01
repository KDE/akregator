/*
   Copyright (C) 2016-2020 Laurent Montel <montel@kde.org>

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
    Q_PROPERTY(Akregator::ArticleGrantleeObject::ArticleStatus articleStatus READ articleStatus)
    Q_PROPERTY(bool important READ important)
    Q_PROPERTY(QString actionToken READ actionToken)

public:
    explicit ArticleGrantleeObject(const Article &article, ArticleFormatter::IconOption icon, QObject *parent = nullptr);
    ~ArticleGrantleeObject();

    enum ArticleStatus {
        Unread = 0,
        Read,
        New
    };
    Q_ENUMS(ArticleStatus)

    Akregator::ArticleGrantleeObject::ArticleStatus articleStatus() const;

    Q_REQUIRED_RESULT QString strippedTitle() const;
    Q_REQUIRED_RESULT QString author() const;
    Q_REQUIRED_RESULT QString content() const;
    Q_REQUIRED_RESULT QString articleLinkUrl() const;
    Q_REQUIRED_RESULT QString articlePubDate() const;
    Q_REQUIRED_RESULT QString enclosure() const;
    Q_REQUIRED_RESULT QString articleCompleteStoryLink() const;
    Q_REQUIRED_RESULT QString imageFeed() const;

    //Action
    Q_REQUIRED_RESULT QString deleteAction() const;
    Q_REQUIRED_RESULT QString markAsReadAction() const;
    Q_REQUIRED_RESULT QString markAsUnreadAction() const;
    Q_REQUIRED_RESULT QString markAsImportantAction() const;
    Q_REQUIRED_RESULT QString sendUrlAction() const;
    Q_REQUIRED_RESULT QString sendFileAction() const;

    Q_REQUIRED_RESULT bool important() const;
    Q_REQUIRED_RESULT QString actionToken() const;

private:
    Article mArticle;
    ArticleFormatter::IconOption mArticleFormatOption;
};
}
#endif // ARTICLEGRANTLEEOBJECT_H
