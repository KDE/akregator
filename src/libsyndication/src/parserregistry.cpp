/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "abstractparser.h"
#include "parserregistry.h"

#include <QHash>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {

static KStaticDeleter<ParserRegistry> parserregistrysd;

ParserRegistry* ParserRegistry::m_self = 0;

class ParserRegistry::ParserRegistryPrivate
{
    public:
    QHash<QString, AbstractParser*> parsers;
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

bool ParserRegistry::registerParser(AbstractParser* parser)
{
    if (d->parsers.contains(parser->format()))
        return false;

    d->parsers.insert(parser->format(), parser);
    return true;
}

void ParserRegistry::unregisterParser(AbstractParser* parser)
{
    d->parsers.remove(parser->format());
}

Document* ParserRegistry::parse(const DocumentSource& source, const QString& formatHint)
{
    if (d->parsers.contains(formatHint))
    {
        if (d->parsers[formatHint]->accept(source))
            return d->parsers[formatHint]->parse(source);
    }

    Q_FOREACH (AbstractParser* i, d->parsers)
    {
        if (i->accept(source))
        {
            return i->parse(source);
        }
    }
    return 0;
}

} // namespace LibSyndication
