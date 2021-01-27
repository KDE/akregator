/*
   SPDX-FileCopyrightText: 2019-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DOWNLOADFEEDICONJOB_H
#define DOWNLOADFEEDICONJOB_H

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
    ~DownloadFeedIconJob();
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

#endif // DOWNLOADFEEDICONJOB_H
