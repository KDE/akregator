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

#include "abstractdocument.h"
#include "documentsource.h"
#include "feed.h"
#include "parsercollection.h"

#include <QByteArray>
#include <QFile>
#include <QString>

#include <iostream>


using namespace LibSyndication;

void printUsage(const QString& error)
{
    std::cerr << "testlibsyndication - (C) Frank Osterfeld 2006" << std::endl;
    std::cerr << std::endl;
    if (!error.isNull())
    {
        std::cerr << error.toUtf8().data() << std::endl;
        std::cerr << std::endl;
    }
    else
    {
        std::cerr << "Prints the parsed content of a feed file to standard output." << std::endl;
        std::cerr << std::endl;
    }
    std::cerr << "Usage: testlibsyndication [--specific-format] <file>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "--specific-format: If set, the debug output for the specific" << std::endl;
    std::cerr << "feed format is printed to stdout, otherwise the debug output" << std::endl;
    std::cerr << "for the abstraction" << std::endl;
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printUsage("filename expected");
        return 1;
    }

    QString filename(argv[1]);
    QString arg1(argv[1]);
    
    bool specificformat = false;
    
    if (filename == "--specific-format")
    {
        if (argc < 3)
        {
            printUsage("filename expected");
            return 1;
        }
        filename = QString(argv[2]);
        specificformat = true;
    }
        
    
    QFile f(filename);

    if (!f.open(QIODevice::ReadOnly))
    {
        printUsage("Couldn't open file");
        return 1;
    }
 
    DocumentSource src(f.readAll(), "http://libsyndicationtest");
    f.close();

    FeedPtr ptr(LibSyndication::parse(src));

    if (ptr.isNull())
    {
        printUsage("Couldn't parse file");
        return 1;
    }

    if (specificformat)
    {
        std::cout << ptr->document()->debugInfo().toUtf8().data() << std::endl;
    }
    else
    {
        std::cout << ptr->debugInfo().toUtf8().data() << std::endl;
    }
    
    return 0;
}
