/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "feed.h"
#include "feedscollection.h"

#include <kurl.h>
#include <kdebug.h>

#include <qlistview.h>

using namespace Akregator;
using namespace RSS;

Feed::Feed(QListViewItem *i, FeedsCollection *coll)
    : FeedGroup(i, coll)
    , xmlUrl()
    , htmlUrl()
    , description()
    , isLiveJournal(false)
    , ljUserName()
    , ljAuthMode(AuthNone)
    , ljLogin()
    , ljPassword()
    , updateTitle(false)
    , articles()
{
//    updateView();
}

Feed::~Feed()
{
}

/*void Feed::open(QTextStream &ts)
{
}*/

QString Feed::ljAuthModeStr()
{
    if (ljAuthMode == AuthLocal)
        return "local";
    if (ljAuthMode == AuthGlobal)
        return "global";
    return "none";
}

/*static*/ Feed::LJAuthMode Feed::authModeFromString(const QString &mode)
{
    QString m = mode.lower();
    if (m == "local")
        return AuthLocal;
    if (m == "global")
        return AuthGlobal;
    return AuthNone;
}


void Feed::save(QTextStream &ts, int /*depth*/)
{
    ts << "<outline text=\"" << title() << "\" "
                  "title=\"" << title() << "\" "
                 "xmlUrl=\"" << xmlUrl << "\" "
                "htmlUrl=\"" << htmlUrl << "\" "
            "description=\"" << description << "\" "
          "isLiveJournal=\"" << (isLiveJournal ? "true" : "false") << "\" "
             "ljUserName=\"" << ljUserName << "\" "
             "ljAuthMode=\"" << ljAuthModeStr() << "\" "
                "ljLogin=\"" << ljLogin << "\" "
             "ljPassword=\"" << ljPassword << "\" "
            "updateTitle=\"" << (updateTitle ? "true" : "false") << "\" "
                   "type=\"akrss\" "
                "version=\"RSS\"/>" << endl;
}

void Feed::fetch()
{
    Loader *loader = Loader::create( this, SLOT(fetchCompleted(Loader *, Document, Status)) );
    loader->loadFrom( xmlUrl, new FileRetriever );
}

void Feed::fetchCompleted(Loader */*loader*/, Document doc, Status status)
{
    // Note that Loader::~Loader() is private, so you cannot delete Loader instances.
    // You don't need to do that anyway since Loader instances delete themselves.

    if (status != Success)
        return;

    kdDebug() << "Feed fetched successfully [" << doc.title() << "]" << endl;

    if (updateTitle || title().isEmpty()) setTitle( doc.title() );
    description = doc.description();
    htmlUrl = doc.link().url();
    articles = doc.articles();
    // TODO: more attributes to fetch?

    emit fetched(this);
}

#include "feed.moc"
