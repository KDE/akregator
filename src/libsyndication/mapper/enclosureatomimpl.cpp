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

#include "enclosureatomimpl.h"

#include <QString>

namespace LibSyndication {

EnclosureAtomImpl::EnclosureAtomImpl(const LibSyndication::Atom::Link& link)
    : m_link(link)
{}
        
bool EnclosureAtomImpl::isNull() const
{
    return m_link.isNull();
}

QString EnclosureAtomImpl::url() const
{
    return m_link.href();
}

QString EnclosureAtomImpl::title() const
{
    return m_link.title();
}

QString EnclosureAtomImpl::type() const
{
    return m_link.type();
}

uint EnclosureAtomImpl::length() const
{
    return m_link.length();
}

uint EnclosureAtomImpl::duration() const
{
    return 0;
}
 
} // namespace LibSyndication
