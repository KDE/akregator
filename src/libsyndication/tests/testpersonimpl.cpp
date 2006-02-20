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

#include "testpersonimpl.h"
#include "mapper/personimpl.h"
#include "person.h"

#include <QList>
#include <QString>
#include <QStringList>

using LibSyndication::Person;
using LibSyndication::PersonPtr;
using LibSyndication::PersonImpl;

void TestPersonImpl::fromString()
{
    QStringList s;
    QList<PersonPtr> p;

    
    
    s.append(QString());
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QString())));
    
    s.append("");
    p.append(PersonPtr(new PersonImpl(QString(), QString(), QString())));
    
    s.append("foo@bar.com");
    p.append(PersonPtr(new PersonImpl(QString(), QString(), "foo@bar.com")));
    
    s.append("<foo@bar.com>");
    p.append(PersonPtr(new PersonImpl(QString(), QString(), "foo@bar.com")));
    
    s.append("Foo");
    p.append(PersonPtr(new PersonImpl("Foo", QString(), QString())));
    
    s.append("Foo M. Bar");
    p.append(PersonPtr(new PersonImpl("Foo M. Bar", QString(), QString())));
    
    s.append("Foo <foo@bar.com>");
    p.append(PersonPtr(new PersonImpl("Foo", QString(), "foo@bar.com")));
    
    s.append("Foo Bar <foo@bar.com>");
    p.append(PersonPtr(new PersonImpl("Foo Bar", QString(), "foo@bar.com")));

    QList<PersonPtr> q;
    
    QStringList::ConstIterator it = s.begin();
    QStringList::ConstIterator end = s.end();
    QList<PersonPtr>::ConstIterator pit = p.begin();
    
    while (it != end)
    {
        PersonPtr q(PersonImpl::fromString(*it));
        QCOMPARE(q->debugInfo(), (*pit)->debugInfo());
        ++it;
        ++pit;
    }
}

QTEST_MAIN(TestPersonImpl);

#include "testpersonimpl.moc"
