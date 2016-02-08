/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program;
  if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "downloadarticlejob.h"
#include <QTemporaryFile>
#include <QDesktopServices>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <QDebug>
#include <QUrlQuery>
#include <PimCommon/AttachmentTemporaryFilesDirs>

using namespace Akregator;

DownloadArticleJob::DownloadArticleJob(QObject *parent)
    : QObject(parent),
      mTemporaryFile(Q_NULLPTR),
      mAttachmentTemporaryFile(Q_NULLPTR)
{

}

DownloadArticleJob::~DownloadArticleJob()
{
    if (mAttachmentTemporaryFile) {
        qDebug()<<" DownloadArticleJob::~DownloadArticleJob()"<<this;
        mAttachmentTemporaryFile->removeTempFiles();
        mAttachmentTemporaryFile = 0;
    }
}

void DownloadArticleJob::forceCleanupTemporaryFile()
{
    if (mAttachmentTemporaryFile) {
        mAttachmentTemporaryFile->forceCleanTempFiles();
        delete mAttachmentTemporaryFile;
        mAttachmentTemporaryFile = 0;
    }
}

void DownloadArticleJob::start()
{
    if (mArticleUrl.isEmpty()) {
        deleteLater();
        return;
    }
    if (mTemporaryFile) {
        qDebug() << " There is a problem as we call start twice";
        return;
    }
    mTemporaryFile = new QTemporaryFile(this);
    mTemporaryFile->open();
    mTemporaryFile->setAutoRemove(false);
    mAttachmentTemporaryFile = new PimCommon::AttachmentTemporaryFilesDirs(this);

    KIO::Job *job = KIO::file_copy(mArticleUrl, QUrl::fromLocalFile(mTemporaryFile->fileName()), -1, KIO::Overwrite);
    mAttachmentTemporaryFile->addTempFile(mTemporaryFile->fileName());
    connect(job, &KIO::Job::result, this, &DownloadArticleJob::slotUrlSaveResult);
}

void DownloadArticleJob::setArticleUrl(const QUrl &articleUrl)
{
    mArticleUrl = articleUrl;
}

void DownloadArticleJob::slotUrlSaveResult(KJob *job)
{
    if (job->error()) {
        KIO::Job *kiojob = dynamic_cast<KIO::Job *>(job);
        if (kiojob && kiojob->ui()) {
            kiojob->ui()->showErrorMessage();
        } else {
            qWarning() << "There is no GUI delegate set for a kjob, and it failed with error:" << job->errorString();
        }
    } else {
        sendAttachment();
        deleteLater();
    }
}

void DownloadArticleJob::setTitle(const QString &title)
{
    mTitle = title;
}

void DownloadArticleJob::sendAttachment()
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("subject"), mTitle);
    query.addQueryItem(QStringLiteral("body"), mText);
    query.addQueryItem(QStringLiteral("attach"), mTemporaryFile->fileName());
    QUrl url;
    url.setScheme(QStringLiteral("mailto"));
    url.setQuery(query);
    QDesktopServices::openUrl(url);
    deleteLater();
}

void DownloadArticleJob::setText(const QString &text)
{
    mText = text;
}
