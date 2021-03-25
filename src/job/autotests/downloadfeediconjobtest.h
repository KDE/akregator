/*
   SPDX-FileCopyrightText: 2019-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class DownloadFeedIconJobTest : public QObject
{
    Q_OBJECT
public:
    explicit DownloadFeedIconJobTest(QObject *parent = nullptr);
    ~DownloadFeedIconJobTest() = default;
private Q_SLOTS:
    void shouldHaveDefaultValues();
};

