/*
    This file is part of Akregator.

    Copyright (C) 2018 Daniel Vrátil <dvratil@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "feedretriever.h"
#include "akregatorconfig.h"
#include "akregator-version.h"

#include <KIO/StoredTransferJob>

#include <QUrl>

using namespace Akregator;

FeedRetriever::FeedRetriever()
    : Syndication::DataRetriever()
{
}

void FeedRetriever::retrieveData(const QUrl &url)
{
    QString userAgent = QStringLiteral("Akregator/%1; syndication").arg(QStringLiteral(AKREGATOR_VERSION));
    if (!Settings::customUserAgent().isEmpty()) {
        userAgent = Settings::customUserAgent();
    }
    bool useCache = Settings::useHTMLCache();

    auto job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    job->addMetaData(QStringLiteral("UserAgent"), userAgent);
    job->addMetaData(QStringLiteral("cache"), useCache ? QStringLiteral("refresh") : QStringLiteral("reload"));
    connect(job, &KJob::result, this, &FeedRetriever::getFinished);
    mJob = job;
    mJob->start();
}

int FeedRetriever::errorCode() const
{
    return mError;
}

void FeedRetriever::abort()
{
    if (mJob) {
        mJob->kill();
        mJob = nullptr;
    }
}

void FeedRetriever::getFinished(KJob *job)
{
    if (job->error()) {
        mError = job->error();
        Q_EMIT dataRetrieved({}, false);
        return;
    }

    Q_EMIT dataRetrieved(static_cast<KIO::StoredTransferJob*>(job)->data(), true);
}
