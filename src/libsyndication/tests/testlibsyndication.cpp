/*
    This file is part of LibSyndication.

    Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "documentsource.h"
#include "feed.h"
#include "parsercollection.h"

#include <QByteArray>
#include <QFile>
#include <QString>

#include <iostream>


using namespace LibSyndication;

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        std::cerr << "filename expected" << std::endl;
        return 1;
    }

    QFile f(argv[1]);

    if (!f.open(QIODevice::ReadOnly))
    {
        std::cerr << "Couldn't open file" << std::endl;
        return 1;
    }
 
    DocumentSource src(f.readAll(), "http://libsyndicationtest");
    f.close();
    FeedPtr ptr(ParserCollection::self()->parse(src));
    if (ptr.isNull())
    {
        std::cerr << "Couldn't parse file" << std::endl;
        return 1;
    }

    std::cout << ptr->debugInfo().toUtf8().data() << std::endl;
    return 0;
}
