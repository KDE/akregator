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

#ifndef DOWNLOADFEEDICONJOB_H
#define DOWNLOADFEEDICONJOB_H

#include <QObject>

#include "akregator_export.h"
namespace Akregator {
class AKREGATOR_EXPORT DownloadFeedIconJob : public QObject
{
    Q_OBJECT
public:
    explicit DownloadFeedIconJob(QObject *parent = nullptr);
    ~DownloadFeedIconJob();
    Q_REQUIRED_RESULT bool start();
    Q_REQUIRED_RESULT bool canStart() const;

    Q_REQUIRED_RESULT QString feedIconUrl() const;
    void setFeedIconUrl(const QString &feedIconUrl);

    Q_REQUIRED_RESULT bool downloadFavicon() const;
    void setDownloadFavicon(bool downloadFavicon);

Q_SIGNALS:
    void result(const QString &fileName);
private:
    QString mFeedIconUrl;
    bool mDownloadFavicon = true;
};
}

#endif // DOWNLOADFEEDICONJOB_H
