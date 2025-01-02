/*
   SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "article.h"
#include "articleformatter.h"
#include <QObject>
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

    enum ArticleStatus {
        Unread = 0,
        Read,
        New
    };
    Q_ENUMS(ArticleStatus)

    Akregator::ArticleGrantleeObject::ArticleStatus articleStatus() const;

    [[nodiscard]] QString strippedTitle() const;
    [[nodiscard]] QString author() const;
    [[nodiscard]] QString content() const;
    [[nodiscard]] QString articleLinkUrl() const;
    [[nodiscard]] QString articlePubDate() const;
    [[nodiscard]] QString enclosure() const;
    [[nodiscard]] QString articleCompleteStoryLink() const;
    [[nodiscard]] QString imageFeed() const;

    [[nodiscard]] bool important() const;
    [[nodiscard]] QString actionToken() const;

private:
    const Article mArticle;
    const ArticleFormatter::IconOption mArticleFormatOption;
};
}
