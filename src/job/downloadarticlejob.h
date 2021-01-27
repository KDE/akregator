/*
  SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DOWNLOADARTICLEJOB_H
#define DOWNLOADARTICLEJOB_H

#include <QObject>
#include <QUrl>

#include "akregator_export.h"
class QTemporaryFile;
class KJob;
namespace MimeTreeParser
{
class AttachmentTemporaryFilesDirs;
}
namespace Akregator
{
class AKREGATOR_EXPORT DownloadArticleJob : public QObject
{
    Q_OBJECT
public:
    explicit DownloadArticleJob(QObject *parent = nullptr);
    ~DownloadArticleJob();

    void start();

    void setArticleUrl(const QUrl &articleUrl);
    void setTitle(const QString &title);

    void forceCleanupTemporaryFile();

    void setText(const QString &text);

private Q_SLOTS:
    void slotUrlSaveResult(KJob *job);

private:
    void sendAttachment();
    QUrl mArticleUrl;
    QString mTitle;
    QString mText;
    QTemporaryFile *mTemporaryFile = nullptr;
    MimeTreeParser::AttachmentTemporaryFilesDirs *mAttachmentTemporaryFile = nullptr;
};
}
#endif // DOWNLOADARTICLEJOB_H
