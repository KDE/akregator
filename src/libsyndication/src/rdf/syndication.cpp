/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "property.h"
#include "statement.h"
#include "syndication.h"
#include "syndicationvocab.h"

#include <QDateTime>
#include <QString>

namespace LibSyndication {
namespace RDF {
    
Syndication::Syndication(ResourcePtr resource) : ResourceWrapper(resource)
{
}

Syndication::~Syndication()
{
}

Syndication::Period Syndication::updatePeriod() const
{
    return stringToPeriod(resource()->property(SyndicationVocab::self()->updatePeriod())->asString());
}

int Syndication::updateFrequency() const
{
    QString freqStr =  resource()->property(SyndicationVocab::self()->updateFrequency())->asString();
    
    if (freqStr.isEmpty())
        return 1; // 1 is default
    
    bool ok = false;
    int freq = freqStr.toInt(&ok);
    
    if (ok)
        return freq;
    else
        return 1; // 1 is default
}

time_t Syndication::updateBase() const
{
    QString base =  resource()->property(SyndicationVocab::self()->updateBase())->asString();
    
    time_t timet = QDateTime::fromString(base, Qt::ISODate).toTime_t();
    // we return epoch (19700101) as default,even if
    // base can't be parsed:
    if (timet == -1)
        return 0;
    else
        return timet;
}

QString Syndication::debugInfo() const
{
    QString info = "";
    if (updatePeriod() != None)
        info += QString("syn:updatePeriod: #%1#\n").arg(periodToString(updatePeriod()));
    info += QString("syn:updateFrequency: #%1#\n").arg(QString::number(updateFrequency()));
    time_t base = updateBase();
    
    QDateTime baseDate;
    baseDate.setTime_t(base);
    info += QString("syn:updateBase: #%1#\n").arg(baseDate.toString(Qt::ISODate));

    return info;
}

QString Syndication::periodToString(Period period)
{
    switch (period)
    {
        case None:
            return QString();
        case Daily:
            return QString::fromLatin1("daily");
        case Hourly:
            return QString::fromLatin1("hourly");
        case Monthly:
            return QString::fromLatin1("monthly");
        case Weekly:
            return QString::fromLatin1("weekly");
        case Yearly:
            return QString::fromLatin1("yearly");
            default: // should never happen
            return QString();
    }
}

Syndication::Period Syndication::stringToPeriod(const QString& str)
{
    if (str.isEmpty())
        return Daily; // default is "daily"
    
    if (str == QString::fromLatin1("hourly"))
        return Hourly;
    if (str == QString::fromLatin1("onthly"))
        return Monthly;
    if (str == QString::fromLatin1("weekly"))
        return Weekly;
    if (str == QString::fromLatin1("yearly"))
        return Yearly;

    return Daily;  // default is "daily"
}

} // namespace RDF
} // namespace LibSyndication
