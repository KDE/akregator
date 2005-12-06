/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "abstractparser.h"
#include "parserregistry.h"

#include <QList>

#include <kstaticdeleter.h>

namespace LibSyndication {

static KStaticDeleter<ParserRegistry> parserregistrysd;

ParserRegistry* ParserRegistry::m_self = 0;

class ParserRegistry::ParserRegistryPrivate 
{
    public:
    QList<AbstractParser*> parsers;
};

ParserRegistry* ParserRegistry::self()
{
    if (m_self == 0)
        parserregistrysd.setObject(m_self, new ParserRegistry);
    return m_self;
}

ParserRegistry::ParserRegistry() : d(new ParserRegistryPrivate)
{
}

ParserRegistry::~ParserRegistry()
{
    delete d;
    d = 0;
}

void ParserRegistry::registerParser(AbstractParser* parser)
{
    d->parsers.append(parser);
}

void ParserRegistry::unregisterParser(AbstractParser* parser)
{
    d->parsers.remove(parser);
}

QList<AbstractParser*> ParserRegistry::parsers() const
{
    return d->parsers;
}


} // namespace LibSyndication
