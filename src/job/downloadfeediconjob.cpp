/*
   SPDX-FileCopyrightText: 2019-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "downloadfeediconjob.h"

#include <KIO/FavIconRequestJob>

using namespace Akregator;
DownloadFeedIconJob::DownloadFeedIconJob(QObject *parent)
    : QObject(parent)
{
}

DownloadFeedIconJob::~DownloadFeedIconJob() = default;

bool DownloadFeedIconJob::start()
{
    if (!canStart()) {
        deleteLater();
        return false;
    }

    auto job = new KIO::FavIconRequestJob(mFeedIconUrl);
    if (!mDownloadFavicon) {
        job->setIconUrl(mFeedIconUrl);
    }
    connect(job, &KIO::FavIconRequestJob::result, this, [job, this](KJob *) {
        if (!job->error()) {
            Q_EMIT result(job->iconFile());
        }
        deleteLater();
    });
    return true;
}

bool DownloadFeedIconJob::canStart() const
{
    return !mFeedIconUrl.isEmpty();
}

QUrl DownloadFeedIconJob::feedIconUrl() const
{
    return mFeedIconUrl;
}

void DownloadFeedIconJob::setFeedIconUrl(const QUrl &feedIconUrl)
{
    mFeedIconUrl = feedIconUrl;
}

bool DownloadFeedIconJob::downloadFavicon() const
{
    return mDownloadFavicon;
}

void DownloadFeedIconJob::setDownloadFavicon(bool downloadFavicon)
{
    mDownloadFavicon = downloadFavicon;
}

#include "moc_downloadfeediconjob.cpp"
