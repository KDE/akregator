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
#include <qdom.h>

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

bool Feed::isGroup()
{
    return false;
}

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


QDomElement Feed::toXml( QDomElement parent, QDomDocument document )
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    el.setAttribute( "title", title() );
    el.setAttribute( "xmlUrl", xmlUrl );
    el.setAttribute( "htmlUrl", htmlUrl );
    el.setAttribute( "description", description );
    el.setAttribute( "isLiveJournal", (isLiveJournal ? "true" : "false") );
    el.setAttribute( "ljUserName", ljUserName );
    el.setAttribute( "ljAuthMode", ljAuthModeStr() );
    el.setAttribute( "ljLogin", ljLogin );
    el.setAttribute( "ljPassword", ljPassword );
    el.setAttribute( "updateTitle", (updateTitle ? "true" : "false") );
    el.setAttribute( "type", "akrss" );
    el.setAttribute( "version", "RSS" );
    parent.appendChild( el );
    return el;
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
