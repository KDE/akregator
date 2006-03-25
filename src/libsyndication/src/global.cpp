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

#include "documentsource.h"
#include "global.h"
#include "parsercollectionimpl.h"
#include "mapper/mapperatomimpl.h"
#include "mapper/mapperrdfimpl.h"
#include "mapper/mapperrss2impl.h"
#include "atom/parser.h"
#include "rdf/parser.h"
#include "rss2/parser.h"

#include <kstaticdeleter.h>

namespace LibSyndication {

static ParserCollection<Feed>* parserColl = 0L;
static KStaticDeleter<ParserCollection<Feed> > parsercollsd;

ParserCollection<Feed>* parserCollection()
{
    if (parserColl == 0)
    {
        parsercollsd.setObject(parserColl, new ParserCollectionImpl<LibSyndication::Feed>);
        parserColl->registerParser(new RSS2::Parser, new RSS2Mapper);
        parserColl->registerParser(new RDF::Parser, new RDFMapper);
        parserColl->registerParser(new Atom::Parser, new AtomMapper);
    }
    return parserColl;
}

FeedPtr parse(const DocumentSource& src, const QString& formatHint)
{
    return parserCollection()->parse(src, formatHint);
}

} // namespace LibSyndication

