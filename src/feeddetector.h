/***************************************************************************
 *   Copyright (C) 2004 by Frank Osterfeld                                 *
 *   frank.osterfeld AT kdemail.net                                        *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
 
#ifndef AKREGATORFEEDDETECTOR_H
#define AKREGATORFEEDDETECTOR_H

#include <qstring.h>
#include <qvaluelist.h>

namespace Akregator
{

	class FeedDetectorEntry
	{
		public:
			FeedDetectorEntry() {}
			FeedDetectorEntry(const QString& url, const QString& title) 
			: m_url(url), m_title(title) {}
					
			const QString& url() const { return m_url; } 
			const QString& title() const { return m_title; }
						
		private:	
			const QString m_url;
			const QString m_title;
	};		

	typedef QValueList<FeedDetectorEntry> FeedDetectorEntryList; 
		
	/** a class providing functions to detect linked feeds in HTML sources */
	class FeedDetector
	{	
		public:
			/** \brief searches an HTML page for feeds listed in @c <link> tags
			@c <link> tags with @c rel attribute values @c alternate or 
			@c service.feed are considered as feeds 
			@param s the html source to scan (the actual source, no URI)
			@return a list containing the detected feeds
			*/
			static FeedDetectorEntryList extractFromLinkTags(const QString& s);
			
			/** \brief searches an HTML page for slightly feed-like looking links and catches everything not running away quickly enough. 
			Extracts links from @c <a @c href> tags which contain @c xml, @c rss or @c rdf
			@param s the html source to scan (the actual source, no URI)
			@return a list containing the detected feeds
			*/
			static FeedDetectorEntryList extractBruteForce(const QString& s) 
			{ return FeedDetectorEntryList(); }
						   
			
		private:
			FeedDetector() {}
	};
}

#endif //AKREGATORFEEDDETECTOR_H
