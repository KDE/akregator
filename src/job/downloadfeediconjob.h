/*
   SPDX-FileCopyrightText: 2019-2026 Laurent Montel <montel@kde.org>

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
    [[nodiscard]] bool start();
    [[nodiscard]] bool canStart() const;

    [[nodiscard]] QUrl feedIconUrl() const;
    void setFeedIconUrl(const QUrl &feedIconUrl);

    [[nodiscard]] bool downloadFavicon() const;
    void setDownloadFavicon(bool downloadFavicon);

Q_SIGNALS:
    void result(const QString &fileName);

private:
    QUrl mFeedIconUrl;
    bool mDownloadFavicon = true;
};
}
