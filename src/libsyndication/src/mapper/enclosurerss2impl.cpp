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

#include "enclosurerss2impl.h"

#include <QString>

namespace LibSyndication {

EnclosureRSS2Impl::EnclosureRSS2Impl(const LibSyndication::RSS2::Enclosure& enclosure)
    : m_enclosure(enclosure)
{}
        
bool EnclosureRSS2Impl::isNull() const
{
    return m_enclosure.isNull();
}

QString EnclosureRSS2Impl::url() const
{
    return m_enclosure.url();
}

QString EnclosureRSS2Impl::title() const
{
    // RSS2 enclosures have no title
    return QString::null;
}

QString EnclosureRSS2Impl::type() const
{
    return m_enclosure.type();
}

uint EnclosureRSS2Impl::length() const
{
    return m_enclosure.length();
}
 
} // namespace LibSyndication
