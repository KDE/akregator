/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program;
  if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef DOWNLOADARTICLEJOB_H
#define DOWNLOADARTICLEJOB_H

#include <QObject>
#include <QUrl>
class QTemporaryFile;
class KJob;
namespace PimCommon
{
class AttachmentTemporaryFilesDirs;
}
namespace Akregator
{
class DownloadArticleJob : public QObject
{
    Q_OBJECT
public:
    explicit DownloadArticleJob(QObject *parent = Q_NULLPTR);
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
    QTemporaryFile *mTemporaryFile;
    PimCommon::AttachmentTemporaryFilesDirs *mAttachmentTemporaryFile;
};
}
#endif // DOWNLOADARTICLEJOB_H
