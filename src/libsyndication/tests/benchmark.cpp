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

#include "specificdocument.h"
#include "documentsource.h"
#include "feed.h"
#include "parsercollection.h"

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QTextStream>

#include <iostream>

#include <cstdio>

using namespace LibSyndication;

int main(int argc, char **argv)
{
    QString filename(argv[1]);
    QString arg1(argv[1]);
    
    /*
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
    */  
    
    int totalLength = 0;
    int numberOfFiles = 0;
    
    QStringList filenames;
    
    QFile input(arg1);
    input.open(QIODevice::ReadOnly);
    QTextStream stream(&input);
    
    while (!stream.atEnd())
    {
        filenames += stream.readLine();
    }
    input.close();

    QTime time;
    time.start();
    
    foreach (QString filename, filenames)
    {
       
        QFile f(filename);
    
        if (!f.open(QIODevice::ReadOnly))
            continue;
        
        DocumentSource src(f.readAll(), "http://libsyndicationtest");
        f.close();
        
        
        FeedPtr ptr(LibSyndication::parse(src));
        if (ptr)
        {
            totalLength += src.size();
            ++numberOfFiles;
            QString dbg = ptr->debugInfo();
        }
        
        // std::cerr << "size of debug output: " << dbg.length() << std::endl;
    }
    
    int timeSpent = time.elapsed();
    
    std::cout << "total number of files: " << numberOfFiles << std::endl;
    std::cout << "total length of source (bytes): " << totalLength << std::endl;
    std::cout << "avg length (bytes): " << (double)totalLength / numberOfFiles << std::endl;
    std::cout << "time needed in total (ms): " << timeSpent << std::endl;
    std::cout << "source bytes per second: " << (totalLength*1000.0) / timeSpent << std::endl;
    std::cout << "files per second: " << ((double)numberOfFiles * 1000) / timeSpent << std::endl;
    return 0;
}
