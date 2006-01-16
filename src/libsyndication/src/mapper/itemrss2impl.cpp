
/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "itemrss2impl.h"

#include <QDateTime>
#include <QList>
#include <QString>

namespace LibSyndication {
namespace Mapper {

ItemRSS2Impl::ItemRSS2Impl(const LibSyndication::RSS2::Item& item) : m_item(item)
{
}

QString ItemRSS2Impl::title() const
{
    return m_item.title();
}

QString ItemRSS2Impl::link() const
{
    
    QString guid = m_item.guid();
    if (!guid.isEmpty() && m_item.guidIsPermaLink())
        return guid;
    else
        return m_item.link();
}

QString ItemRSS2Impl::description() const
{
    return m_item.description();
}

QString ItemRSS2Impl::content() const
{
    return m_item.content();
}

QString ItemRSS2Impl::author() const
{
    return m_item.author();
}

QString ItemRSS2Impl::language() const
{
    return "TODO";
}

QString ItemRSS2Impl::id() const
{
    QString guid = m_item.guid();
    if (guid.isEmpty())
    {
        // TODO: calc hash
        return QString::null;
    }
    else
        return guid;
}

time_t ItemRSS2Impl::datePublished() const
{
    time_t time = m_item.pubDate().toTime_t();
    if (time == -1) 
        return 0;
    else 
        return time;
}

time_t ItemRSS2Impl::dateUpdated() const
{
    return datePublished();
}

//LibSyndication::Enclosure ItemRSS2Impl::enclosure() const
//{
//}

} // namespace Mapper
} // namespace LibSyndication
