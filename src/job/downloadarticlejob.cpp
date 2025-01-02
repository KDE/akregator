/*
  SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "downloadarticlejob.h"
#include "akregator_debug.h"
#include <KIO/FileCopyJob>
#include <KIO/JobUiDelegate>
#include <MimeTreeParser/AttachmentTemporaryFilesDirs>
#include <QDesktopServices>
#include <QTemporaryFile>
#include <QUrlQuery>

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
        auto kiojob = dynamic_cast<KIO::Job *>(job);
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

#include "moc_downloadarticlejob.cpp"
