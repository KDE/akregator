/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
 
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <kcharsets.h>
#include <kurl.h>

#include "feeddetector.h"


using namespace RSS;

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

        // we accept only type attributes indicating a feed
        if ( type != "application/rss+xml" && type != "application/rdf+xml"
	      && type != "application/atom+xml" && type != "text/xml" )
            continue;
                
        QString title;
        pos = reTitle.search(*it, 0);
        if (pos != -1)
        title = reTitle.cap(1);

        title = KCharsets::resolveEntities(title);

        QString url;
        pos = reHref.search(*it, 0);
        if (pos != -1)
            url = reHref.cap(1);

        url = KCharsets::resolveEntities(url);

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

                url = KCharsets::resolveEntities(url);

                if ( rssrdfxml.exactMatch(url) )
                    list.append(url);
            }

            pos = matchpos + reAhrefTag.matchedLength();
        }
    }
    
    return list;
}

QString FeedDetector::fixRelativeURL(const QString &s, const KURL &baseurl)
{
    QString s2=s;
    KURL u;
    if (KURL::isRelativeURL(s2))
    {
        if (s2.startsWith("//"))
        {
            s2=s2.prepend(baseurl.protocol()+":");
            u=s2;
        }
        else if (s2.startsWith("/"))
        {
            KURL b2(baseurl);
            b2.setPath(QString()); // delete path and query, so that only protocol://host remains
            b2.setQuery(QString());
            u = KURL(b2, s2.remove(0,1)); // remove leading "/" 
        }
        else
        {
            u = KURL(baseurl, s2);
        }
    }
    else
        u=s2;

    u.cleanPath();
    //kdDebug() << "AKREGATOR_PLUGIN_FIXURL: " << "url=" << s << " baseurl=" << baseurl.url() << " fixed=" << u.url() << 
    //endl;
    return u.url();
}
