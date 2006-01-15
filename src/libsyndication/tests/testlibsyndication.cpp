/*
    This file is part of LibSyndication.

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

#include "abstractdocument.h"
#include "documentsource.h"
#include "parserregistry.h"
#include "atom/parser.h"
#include "rdf/parser.h"
#include "rss2/parser.h"

#include <kinstance.h>

#include <QByteArray>
#include <QFile>
#include <QString>

#include <iostream>

using namespace LibSyndication;

int main(int argc, char **argv)
{
    ParserRegistry::self()->registerParser(new RSS2::Parser);
    ParserRegistry::self()->registerParser(new RDF::Parser);
    ParserRegistry::self()->registerParser(new Atom::Parser);

    if (argc < 2)
    {
        std::cerr << "filename expected" << std::endl;
        return 1;
    }

    QString filename(argv[1]);
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    if (file.error() != QFile::NoError)
    {
        std::cerr << "Couldn't read file: " << file.errorString().toLocal8Bit().data() << std::endl;
    }

    QByteArray raw = file.readAll();
    file.close();

    DocumentSource src(raw);
    AbstractDocumentPtr doc = ParserRegistry::self()->parse(src);

    if (doc)
    {
         std::cout << doc->debugInfo().toLocal8Bit().data() << std::endl;
    }
    else
    {
         std::cout << "Couldn't parse document " << argv[1] << std::endl;
         return 1;
    }
}

