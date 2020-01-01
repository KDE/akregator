/*
  Copyright (C) 2016-2020 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program;
  if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "downloadarticlejob.h"
#include "akregator_debug.h"
#include <QTemporaryFile>
#include <QDesktopServices>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <QUrlQuery>
#include <MimeTreeParser/AttachmentTemporaryFilesDirs>

using namespace Akregator;

DownloadArticleJob::DownloadArticleJob(QObject *parent)
    : QObject(parent)
{
}

DownloadArticleJob::~DownloadArticleJob()
{
    if (mAttachmentTemporaryFile) {
        mAttachmentTemporaryFile->removeTempFiles();
        mAttachmentTemporaryFile = nullptr;
    }
}

void DownloadArticleJob::forceCleanupTemporaryFile()
{
    if (mAttachmentTemporaryFile) {
        mAttachmentTemporaryFile->forceCleanTempFiles();
        delete mAttachmentTemporaryFile;
        mAttachmentTemporaryFile = nullptr;
    }
}

void DownloadArticleJob::start()
{
    if (mArticleUrl.isEmpty()) {
        deleteLater();
        return;
    }
    if (mTemporaryFile) {
        qCDebug(AKREGATOR_LOG) << " There is a problem as we call start twice";
        return;
    }
    mTemporaryFile = new QTemporaryFile(this);
    mTemporaryFile->open();
    mTemporaryFile->setAutoRemove(false);
    mAttachmentTemporaryFile = new MimeTreeParser::AttachmentTemporaryFilesDirs;

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
        if (kiojob && kiojob->uiDelegate()) {
            kiojob->uiDelegate()->showErrorMessage();
        } else {
            qCWarning(AKREGATOR_LOG) << "There is no GUI delegate set for a kjob, and it failed with error:" << job->errorString();
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
