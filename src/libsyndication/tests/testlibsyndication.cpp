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

#include "testlibsyndication.h"

#include "abstractdocument.h"
#include "feed.h"
#include "loader.h"
#include "atom/parser.h"
#include "rdf/parser.h"
#include "rss2/parser.h"

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kinstance.h>
#include <klocale.h>
#include <kurl.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QString>

#include <cstdlib>
#include <iostream>

using namespace LibSyndication;

static const KCmdLineOptions options[] =
{
    { "+url", I18N_NOOP("URL of feed"), 0 },
    KCmdLineLastOption
};

TestLibSyndication::TestLibSyndication(const QString& url)
{
    KUrl kurl;
    if (!KUrl::isRelativeURL(url))
        kurl = KUrl(url);
    else
        kurl = KUrl("file://"+QDir::currentPath(), url);
        
    
    Loader* loader = Loader::create(this, SLOT(slotLoadingComplete(LibSyndication::Loader*,
                                    LibSyndication::FeedPtr,
                                    LibSyndication::ErrorCode)));
    loader->loadFrom(kurl);
}

void TestLibSyndication::slotLoadingComplete(LibSyndication::Loader* loader,
                    LibSyndication::FeedPtr feed,
                    LibSyndication::ErrorCode error)
{
    if (feed)
    {
        std::cout << feed->debugInfo().toLocal8Bit().data() << std::endl;
        exit(0);
    }
    else
    {
        std::cout << "error" << std::endl;
        exit(1);
    }
    
}
                    
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "filename expected" << std::endl;
        return 1;
    }

    KAboutData aboutData("testlibsyndication", "testlibsyndication", "0.1");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() != 1 ) args->usage();

    TestLibSyndication* tester = new TestLibSyndication(args->arg( 0 ));
    
    return app.exec();
}

#include "testlibsyndication.moc"
