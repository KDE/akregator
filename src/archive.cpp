/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "archive.h"
#include "feed.h"
#include "feediconmanager.h"

#include <kurl.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include <qdom.h>
#include <qpixmap.h>
#include <qfile.h>

using namespace Akregator;

void Archive::load(Feed *f)
{
    if (!f)
        return;
    
    if (f->isMerged() || f->xmlUrl().isNull())
        return;
   
    KURL url( f->xmlUrl() );
    
    if (!url.isValid())
        return;
    
    QString iconFile = FeedIconManager::self()->iconLocation("http://"+url.host());
    
    if (!iconFile.isNull())
        f->setFavicon(QPixmap(KGlobal::dirs()->findResource("cache", iconFile+".png")));

    // images are cache, articles is data.. good I think.
    QString u=f->xmlUrl();
    
    QString imageFileName=KGlobal::dirs()->saveLocation("cache", "akregator/Media/")+u.replace("/", "_").replace(":", "_")+".png";
    f->setImage(QPixmap(imageFileName, "PNG")); 
    
    QString filePath = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + url.prettyURL(-1).replace("/", "_").replace(":", "_") + ".xml";

    //kdDebug() << "Will read feed from " << filePath << endl;
    //kdDebug() << "merged :"<<f->isMerged()<<endl;

    f->setMerged(true); // so it is merged even if we error _from now on_

   // if archiving is disabled for this feed, don't load the archive    
   if (  (f->archiveMode() == Feed::globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::disableArchiving) || f->archiveMode() == Feed::disableArchiving) 
        return;           
            
    QFile file(filePath);
    
    if ( !file.open( IO_ReadOnly ) ) {
        return;
    }
    
    QTextStream stream( &file );
    stream.setEncoding(QTextStream::UnicodeUTF8);
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

    // if archiving is disabled for this feed, don't save the archive
            
    if (  (f->archiveMode() == Feed::globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::disableArchiving) || f->archiveMode() == Feed::disableArchiving) 
        return;           
        

    KURL url( f->xmlUrl() );

    QString filePath = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + url.prettyURL(-1).replace("/", "_").replace(":", "_") + ".xml";

    //kdDebug() << "Will save feed to " << filePath << endl;

    QFile file(filePath);
    
    if ( !file.open( IO_WriteOnly ) ) {
        kdDebug() << "load error"<<endl;
        return;
    }
    
    QTextStream stream( &file );
    stream.setEncoding(QTextStream::UnicodeUTF8);
    
    QDomDocument doc;
    QDomProcessingInstruction z = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild( z );
    
    QDomElement root = doc.createElement( "rss" );
    root.setAttribute("version","2.0");
    root.setAttribute("xmlns:metaInfo","http://foobar");
    doc.appendChild( root );
    f->dumpXmlData( root, doc);
    
    stream<<doc.toString();
    //kdDebug()<< "dump="<<doc.toString()<<endl;
    
}

