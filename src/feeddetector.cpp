/*
    This file is part of Akregator.

    Copyright (C) 2004 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
 
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "feeddetector.h"


using namespace Akregator;

FeedDetectorEntryList FeedDetector::extractFromLinkTags(const QString& s)	
{
    //reduce all sequences of spaces, newlines etc. to one space:
    QString str = s.simplifyWhiteSpace();

    // extracts <link> tags
    QRegExp reLinkTag("<[\\s]?LINK[^>]*REL[\\s]?=[\\s]?\\\"[\\s]?(ALTERNATE|SERVICE\\.FEED)[\\s]?\\\"[^>]*>", false);

    // extracts the URL (href="url")
    QRegExp reHref("HREF[\\s]?=[\\s]?\\\"([^\\\"]*)\\\"", false);
    // extracts type attribute
    QRegExp reType("TYPE[\\s]?=[\\s]?\\\"([^\\\"]*)\\\"", false);
    // extracts the title (title="title")
    QRegExp reTitle("TITLE[\\s]?=[\\s]?\\\"([^\\\"]*)\\\"", false);

    int pos = 0;
    int matchpos = 0;

    // get all <link> tags
    QStringList linkTags;
    //int strlength = str.length();
    while ( matchpos != -1 )
    {
        matchpos = reLinkTag.search(str, pos);
        if (matchpos != -1)
        {
            linkTags.append( str.mid(matchpos, reLinkTag.matchedLength()) );
            pos = matchpos + reLinkTag.matchedLength();
        }
    }

    FeedDetectorEntryList list;

    for ( QStringList::Iterator it = linkTags.begin(); it != linkTags.end(); ++it )
    {
        QString type;
        int pos = reType.search(*it, 0);
        if (pos != -1)
            type = reType.cap(1).lower();

        // we accept only type attributes indicating a feed or omitted type attributes
        if ( !type.isEmpty() && type != "application/rss+xml" && type != "application/rdf+xml"
	      && type != "application/atom+xml" && type != "text/xml" )
            continue;
                
        QString title;
        pos = reTitle.search(*it, 0);
        if (pos != -1)
        title = reTitle.cap(1);

        QString url;
        pos = reHref.search(*it, 0);
        if (pos != -1)
            url = reHref.cap(1);

        // if feed has no title, use the url as preliminary title (until feed is parsed)
        if ( title.isEmpty() )
            title = url;

        if ( !url.isEmpty() )
            list.append(FeedDetectorEntry(url, title) );		
    }

    return list;
}

QStringList FeedDetector::extractBruteForce(const QString& s)
{
    QString str = s.simplifyWhiteSpace();
    
    QRegExp reAhrefTag("<[\\s]?A[^>]?HREF=[\\s]?\\\"[^\\\"]*\\\"[^>]*>", false);
    
    // extracts the URL (href="url")
    QRegExp reHref("HREF[\\s]?=[\\s]?\\\"([^\\\"]*)\\\"", false);

    QRegExp rssrdfxml(".*(RSS|RDF|XML)", false);

    int pos = 0;
    int matchpos = 0;
    
    // get all <a href> tags and capture url
    QStringList list;
    //int strlength = str.length();
    while ( matchpos != -1 )
    {
        matchpos = reAhrefTag.search(str, pos);
        if ( matchpos != -1 )
        {
            QString ahref = str.mid(matchpos, reAhrefTag.matchedLength());
            int hrefpos = reHref.search(ahref, 0);
            if ( hrefpos != -1 )
            {
                QString url = reHref.cap(1);
                if ( rssrdfxml.exactMatch(url) )
                    list.append(url);
            }

            pos = matchpos + reAhrefTag.matchedLength();
        }
    }
    
    return list;
    
}
