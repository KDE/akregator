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
    //kdDebug() << "merged :"<<f->isMerged()<<endl;

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

    f->setMerged(false);
    f->appendArticles(feedDoc);
    //kdDebug() << "setting merged=true"<<endl;
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
    doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    
    QDomElement root = doc.createElement( "rss" );
    root.setAttribute("version","2.0");
    root.setAttribute("xmlns:metaInfo","http://foobar");
    doc.appendChild( root );
    f->dumpXmlData( root, doc);
    
    stream<<doc.toString();
    //kdDebug()<< "dump="<<doc.toString()<<endl;
    
}

