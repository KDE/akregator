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

#ifndef ARTICLEGRANTLEEOBJECT_H
#define ARTICLEGRANTLEEOBJECT_H

#include <QObject>
#include <article.h>
#include "articleformatter.h"

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
    Q_PROPERTY(QString deleteAction READ deleteAction)
    Q_PROPERTY(QString markAsAction READ markAsAction)
    Q_PROPERTY(QString markAsUnreadAction READ markAsUnreadAction)
    Q_PROPERTY(QString markAsImportantAction READ markAsImportantAction)
    Q_PROPERTY(QString sendUrlAction READ sendUrlAction)
    Q_PROPERTY(QString openInExternalBrowser READ openInExternalBrowser)
    Q_PROPERTY(QString openInBackgroundTab READ openInBackgroundTab)
    Q_PROPERTY(QString share READ share)
    Q_PROPERTY(QString commentLink READ commentLink)
    Q_PROPERTY(QString commentNumber READ commentNumber)
    Q_PROPERTY(QString articleStatus READ articleStatus)

public:
    explicit ArticleGrantleeObject(const QUrl &imageDir, const Article &article, ArticleFormatter::IconOption icon, QObject *parent = Q_NULLPTR);
    ~ArticleGrantleeObject();

    enum ArticleStatus {
        Unread = 0,
        Read,
        New
    };
    Q_ENUMS(ArticleStatus)

    ArticleStatus articleStatus() const;

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
    QString markAsAction() const;
    QString markAsUnreadAction() const;
    QString markAsImportantAction() const;
    QString sendUrlAction() const;
    QString openInExternalBrowser() const;
    QString share() const;

    QString commentLink() const;
    QString commentNumber() const;
    QString openInBackgroundTab() const;
private:
    QString createActionUrl(const QString &actionName) const;
    Article mArticle;
    ArticleFormatter::IconOption mArticleFormatOption;
    QUrl mImageDir;
};
}
#endif // ARTICLEGRANTLEEOBJECT_H
