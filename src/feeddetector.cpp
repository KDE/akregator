
/***************************************************************************
 *   Copyright (C) 2004 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                        *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
 
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
		QString title;
		int tpos = reTitle.search(*it, 0);
		if (tpos != -1)
			title = reTitle.cap(1);
		
		QString url;
		int upos = reHref.search(*it, 0);
		if (upos != -1)
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
