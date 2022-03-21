/*
   SPDX-FileCopyrightText: 2019-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QUrl>

#include "akregator_export.h"
namespace Akregator
{
class AKREGATOR_EXPORT DownloadFeedIconJob : public QObject
{
    Q_OBJECT
public:
    explicit DownloadFeedIconJob(QObject *parent = nullptr);
    ~DownloadFeedIconJob() override;
    Q_REQUIRED_RESULT bool start();
    Q_REQUIRED_RESULT bool canStart() const;

    Q_REQUIRED_RESULT QUrl feedIconUrl() const;
    void setFeedIconUrl(const QUrl &feedIconUrl);

    Q_REQUIRED_RESULT bool downloadFavicon() const;
    void setDownloadFavicon(bool downloadFavicon);

Q_SIGNALS:
    void result(const QString &fileName);

private:
    QUrl mFeedIconUrl;
    bool mDownloadFavicon = true;
};
}
