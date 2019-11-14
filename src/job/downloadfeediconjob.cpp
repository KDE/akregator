/*
   Copyright (C) 2019 Montel Laurent <montel@kde.org>

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

#include "downloadfeediconjob.h"

#include <KIO/FavIconRequestJob>

#include <QUrl>
using namespace Akregator;
DownloadFeedIconJob::DownloadFeedIconJob(QObject *parent)
    : QObject(parent)
{
}

DownloadFeedIconJob::~DownloadFeedIconJob()
{

}

bool DownloadFeedIconJob::start()
{
    if (!canStart()) {
        deleteLater();
        return false;
    }

    KIO::FavIconRequestJob *job = new KIO::FavIconRequestJob(QUrl(mFeedIconUrl));
    if (!mDownloadFavicon) {
        job->setIconUrl(QUrl(mFeedIconUrl));
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

QString DownloadFeedIconJob::feedIconUrl() const
{
    return mFeedIconUrl;
}

void DownloadFeedIconJob::setFeedIconUrl(const QString &feedIconUrl)
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
