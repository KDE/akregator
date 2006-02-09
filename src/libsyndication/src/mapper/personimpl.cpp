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

#include "personimpl.h"

namespace LibSyndication {

PersonPtr PersonImpl::fromString(const QString& str)
{
    if (str.trimmed().isEmpty())
        return m_nullPerson;
    
    QString name;
    QString uri;
    QString email;

    // TODO: implement person parsing
    // extract email address "<?..@..>?"
    // extract uri if there are real-world RSS/RDF feeds
    // containing  homepage links, otherwise ignore
    // put everything that is not email or uri into name
        
    if (name.isEmpty() && email.isEmpty() && uri.isEmpty())
        return m_nullPerson;
       
    return PersonPtr(new PersonImpl(name, uri, email));
}

PersonImpl::PersonImpl() : m_null(true)
{
}

PersonPtr PersonImpl::m_nullPerson(new PersonImpl);

PersonImpl::PersonImpl(const QString& name, const QString& uri,
                       const QString& email) :
        m_null(false), m_name(name), m_uri(uri), m_email(email)
{
}

} // namespace LibSyndication
