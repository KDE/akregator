/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "archive.h"
#include "feed.h"

#include <kstandarddirs.h>
#include <kdebug.h>

using namespace Akregator;

void Archive::load(Feed *f)
{
    KURL url( f->xmlUrl );

    QString file = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + url.prettyURL(-1).replace("/", "_").replace(":", "_") + ".rss";

    kdDebug() << "Will read feed from " << file << endl;
}

void Archive::save(Feed *f)
{
    KURL url( f->xmlUrl );

    QString file = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + url.prettyURL(-1).replace("/", "_").replace(":", "_") + ".rss";

    kdDebug() << "Will save feed to " << file << endl;
}

