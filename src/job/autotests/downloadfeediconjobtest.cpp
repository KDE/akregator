/*
   SPDX-FileCopyrightText: 2019-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "downloadfeediconjobtest.h"
#include "job/downloadfeediconjob.h"
#include <QTest>
QTEST_MAIN(DownloadFeedIconJobTest)
DownloadFeedIconJobTest::DownloadFeedIconJobTest(QObject *parent)
    : QObject(parent)
{
}

void DownloadFeedIconJobTest::shouldHaveDefaultValues()
{
    Akregator::DownloadFeedIconJob job;
    QVERIFY(!job.canStart());
    QVERIFY(job.downloadFavicon());
    QVERIFY(job.feedIconUrl().isEmpty());
}

#include "moc_downloadfeediconjobtest.cpp"
