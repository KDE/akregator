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

#include <qdom.h>
#include <qfile.h>

using namespace Akregator;

void Archive::load(Feed *f)
{
    if (!f)
        return;
    
    KURL url( f->xmlUrl );

    QString filePath = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + url.prettyURL(-1).replace("/", "_").replace(":", "_") + ".xml";

    kdDebug() << "Will read feed from " << filePath << endl;
    kdDebug() << "merged :"<<f->isMerged()<<endl;

    if (f->isMerged())
        return;
    
    QFile file(filePath);
    
    if ( !file.open( IO_ReadOnly ) ) {
        kdDebug() << "load error"<<endl;
        return;
    }
    
    QTextStream stream( &file );
    QString data=stream.read();
    QDomDocument doc;
    
    if (!doc.setContent(data))
    {
        kdDebug() << "load error invalid xml"<<endl;
        return;
    }

    Document feedDoc = Document(doc);
    if (!feedDoc.isValid())
    {
        kdDebug() << "load error invalid feed"<<endl;
        return;
    }

    f->appendArticles(feedDoc);
    f->setMerged(true);

}

void Archive::save(Feed *f)
{
    if (!f)
        return;

    KURL url( f->xmlUrl );

    QString filePath = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + url.prettyURL(-1).replace("/", "_").replace(":", "_") + ".xml";

    kdDebug() << "Will save feed to " << filePath << endl;

    QFile file(filePath);
    
    if ( !file.open( IO_WriteOnly ) ) {
        kdDebug() << "load error"<<endl;
        return;
    }
    
    QTextStream stream( &file );
    
    QDomDocument doc;
    QDomElement root = doc.createElement( "feed" );
    root.setAttribute("version","0.3");
    root.setAttribute("xmlns","http://purl.org/atom/ns#");
    doc.appendChild( root );
    f->dumpXmlData( root, doc);
    
    stream<<doc.toString();
    kdDebug()<< "dump="<<doc.toString()<<endl;
    
}

