/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "archive.h"
#include "feed.h"
#include "feedgroup.h"
#include "feediconmanager.h"
#include "treenode.h"

#include <kurl.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include <qdom.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qptrlist.h>

using namespace Akregator;



void Archive::load(TreeNode* node)
{
    if (!node)
        return;
    
    if (node->isGroup())
    {
        FeedGroup* fg = static_cast<FeedGroup*>(node);
        load_p(fg);
    }
    else
    {
        Feed* f = static_cast<Feed*>(node);
        load_p(f);
    }
}

void Archive::save(TreeNode* node)
{
    if (!node)
        return;
    
    if (node->isGroup())
    {
        FeedGroup* fg = static_cast<FeedGroup*>(node);
        save_p(fg);
    }
    else
    {
        Feed* f = static_cast<Feed*>(node);
        save_p(f);
    }

}

void Archive::load_p(FeedGroup* fg)
{
    QPtrList<TreeNode> children = fg->children();
    for (TreeNode* i = children.first(); i; i = children.next() )
        load(i);
}

void Archive::save_p(FeedGroup* fg)
{
    QPtrList<TreeNode> children = fg->children();
    for (TreeNode* i = children.first(); i; i = children.next() )
        save(i);
}


void Archive::load_p(Feed *f)
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
//   if (  (f->archiveMode() == Feed::globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::disableArchiving) || f->archiveMode() == Feed::disableArchiving) 
//        return;           
            
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

void Archive::save_p(Feed *f)
{
    if (!f)
        return;

    // if archiving is disabled for this feed, don't save the archive
            
//    if (  (f->archiveMode() == Feed::globalDefault && Settings::archiveMode() == Settings::EnumArchiveMode::disableArchiving) || f->archiveMode() == Feed::disableArchiving) 
//        return;
        

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

