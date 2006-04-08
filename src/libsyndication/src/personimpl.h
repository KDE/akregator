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

#ifndef LIBSYNDICATION_PERSONIMPL_H
#define LIBSYNDICATION_PERSONIMPL_H

#include <person.h>
#include <sharedptr.h>

#include <QString>

namespace LibSyndication {

class PersonImpl;
typedef SharedPtr<PersonImpl> PersonImplPtr;

/**
 * @internal
 */
class PersonImpl : public Person
{
    public:

        PersonImpl();
        PersonImpl(const QString& name, const QString& uri, const QString& email);
        
        virtual bool isNull() const { return m_null; }
        
        virtual QString name() const { return m_name; }
        
        virtual QString uri() const { return m_uri; }
        
        virtual QString email() const { return m_email; }

    private:
        
        bool m_null;
        QString m_name;
        QString m_uri;
        QString m_email;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_PERSONIMPL_H
