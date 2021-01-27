/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef FEEDRETRIEVER_H_
#define FEEDRETRIEVER_H_

#include <Syndication/DataRetriever>

class KJob;

namespace Akregator
{
class FeedRetriever : public Syndication::DataRetriever
{
    Q_OBJECT
public:
    FeedRetriever();

    void retrieveData(const QUrl &url) override;
    void abort() override;
    Q_REQUIRED_RESULT int errorCode() const override;

private Q_SLOTS:
    void getFinished(KJob *job);

private:
    KJob *mJob = nullptr;
    int mError = 0;
};
}

#endif
