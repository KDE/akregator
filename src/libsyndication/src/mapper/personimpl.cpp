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

#include <QRegExp>

namespace LibSyndication {

PersonPtr PersonImpl::fromString(const QString& strp)
{
    QString str = strp.trimmed();
    if (str.isEmpty())
        return m_nullPerson;
    
    QString name;
    QString uri;
    QString email;

    // look for something looking like a mail address ( "foo@bar.com", 
    // "<foo@bar.com>") and extract it
    
    QRegExp remail("<?([^@\\s<]+@[^>\\s]+)>?"); // FIXME: user "proper" regexp,
       // search kmail source for it
    
    int pos = remail.indexIn(str);
    if (pos != -1)
    {
        QString all = remail.cap(0);
        email = remail.cap(1);
        str.replace(all, ""); // remove mail address
    }
    
    // simplify the rest and return it as name
    name = str.simplified();
    
    name = name.isEmpty() ? QString() : name;
    email = email.isEmpty() ? QString() : email;
    uri = uri.isEmpty() ? QString() : uri;

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
