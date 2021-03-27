/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "articleformatter.h"
#include <QObject>
#include <article.h>
namespace Akregator
{
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
    ~ArticleGrantleeObject() override;

    enum ArticleStatus { Unread = 0, Read, New };
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

    Q_REQUIRED_RESULT bool important() const;
    Q_REQUIRED_RESULT QString actionToken() const;

private:
    const Article mArticle;
    const ArticleFormatter::IconOption mArticleFormatOption;
};
}
