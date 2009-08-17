/*
 * This file is part of Akregator
 *
 * Copyright (C) 2009 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef AKREGATOR_IMPORTITEMSJOB_H
#define AKREGATOR_IMPORTITEMSJOB_H

#include <KJob>
#include <boost/weak_ptr.hpp>

namespace KRss {
    class NetResource;
}

namespace Akregator {

class ImportItemsJob : public KJob {
    Q_OBJECT
public:

    enum Error {
        ExporterError = KJob::UserDefinedError,
        ImportFailedError
    };

    explicit ImportItemsJob( const boost::weak_ptr<const KRss::NetResource>& res, QObject* parent=0 );
    ~ImportItemsJob();

    QString xmlUrl() const;
    void setXmlUrl( const QString& xmlUrl );

    /* reimp */ void start();

private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void exporterFinished(int, QProcess::ExitStatus))
    Q_PRIVATE_SLOT( d, void exporterError(QProcess::ProcessError))
    Q_PRIVATE_SLOT( d, void importFinished(KJob*) )
};

}

#endif // AKREGATOR_IMPORTITEMSJOB_H

