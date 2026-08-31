/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "feedretriever.h"
#include "akregator-version.h"
#include "akregatorconfig.h"
#include <KIO/StoredTransferJob>

#include <QUrl>

using namespace Qt::Literals::StringLiterals;

using namespace Akregator;

FeedRetriever::FeedRetriever()
    : Syndication::DataRetriever()
{
}

void FeedRetriever::retrieveData(const QUrl &url)
{
    QString userAgent = u"Akregator/%1; syndication"_s.arg(QStringLiteral(AKREGATOR_VERSION));
    if (!Settings::customUserAgent().isEmpty()) {
        userAgent = Settings::customUserAgent();
    }
    bool useCache = Settings::useHTMLCache();

    auto job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    job->addMetaData(u"UserAgent"_s, userAgent);
    job->addMetaData(u"accept"_s, u"application/rss+xml;q=0.9, application/atom+xml;q=0.9, text/*;q=0.8, */*;q=0.7"_s);
    job->addMetaData(u"cache"_s, useCache ? u"refresh"_s : u"reload"_s);
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

    Q_EMIT dataRetrieved(static_cast<KIO::StoredTransferJob *>(job)->data(), true);
}

#include "moc_feedretriever.cpp"
