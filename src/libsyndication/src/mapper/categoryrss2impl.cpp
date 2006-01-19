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

#include "categoryrss2impl.h"

#include <QString>

namespace LibSyndication {
namespace Mapper {

CategoryRSS2Impl::CategoryRSS2Impl(const LibSyndication::RSS2::Category& category)
    : m_category(category)
{}
        
bool CategoryRSS2Impl::isNull() const
{
    return m_category.isNull();
}

QString CategoryRSS2Impl::term() const
{
    return m_category.category();
}

QString CategoryRSS2Impl::scheme() const
{
    return m_category.domain();
}

QString CategoryRSS2Impl::label() const
{
    return QString::null;
}

 
} // namespace Mapper
} // namespace LibSyndication
