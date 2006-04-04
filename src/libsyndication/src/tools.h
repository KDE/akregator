/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef LIBSYNDICATION_TOOLS_H
#define LIBSYNDICATION_TOOLS_H

#include <kdepimmacros.h>

#include <ctime>

class QByteArray;
class QString;

namespace LibSyndication {

/**
 * @internal
 */
unsigned int calcHash(const QString& str);

/**
 * @internal
 */
unsigned int calcHash(const QByteArray& array);

/**
 * @internal
 */
QString calcMD5Sum(const QString& str);

/** date formats supported by date parsers */

enum DateFormat
{
    ISODate, /**< ISO 8601 extended format.
              * (date: "2003-12-13",datetime: "2003-12-13T18:30:02.25", 
              * datetime with timezone: "2003-12-13T18:30:02.25+01:00")
              */
    RFCDate /** RFC 822. (e.g. "Sat, 07 Sep 2002 00:00:01 GMT") */
};

/**
 * parses a date string in ISO 8601 extended format.
 * (date: "2003-12-13",datetime: "2003-12-13T18:30:02.25", 
 * datetime with timezone: "2003-12-13T18:30:02.25+01:00")
 * 
 * @param str a string in ISO 8601 format
 * @return parsed date in seconds since epoch, 0 if no date could
 * be parsed from the string.
 */
KDE_EXPORT
time_t parseISODate(const QString& str);

/**
 * parses a date string as defined in RFC 822.
 * (Sat, 07 Sep 2002 00:00:01 GMT)
 * 
 * @param str a string in RFC 822 format
 * @return parsed date in seconds since epoch, 0 if no date could
 * be parsed from the string.
 */
KDE_EXPORT
time_t parseRFCDate(const QString& str);

/**
 * parses a date string in ISO (see parseISODate()) or RFC 822 (see 
 * parseRFCDate()) format.
 * It tries both parsers and returns the first valid parsing result found (or 0
 * otherwise).
 * To speed up parsing, you can give a hint which format you expect.
 * The method will try the corresponding parser first then.
 * 
 * @param str a date string
 * @param hint the expected format
 * @return parsed date in seconds since epoch, 0 if no date could
 * be parsed from the string.
 */
KDE_EXPORT
time_t parseDate(const QString& str, DateFormat hint=RFCDate);


/**
 * @internal
 * returns a string representation of a datetime.
 * this is used internally to create debugging output.
 * 
 * @param date the date to convert
 * @return string representation of the date, or a null string if 
 * @c date is 0
 */
KDE_EXPORT
QString dateTimeToString(time_t date);

/**
 * resolves entities to respective unicode chars.
 * 
 * @param str a string 
 */
KDE_EXPORT
QString resolveEntities(const QString& str);
   
/**
 * replaces the characters &lt; >, &, ", '
 * with &amp;lt; &amp;gt; &amp;amp;, &amp;quot; &amp;apos;. 
 * @param str the string to escape
 */
KDE_EXPORT
QString escapeSpecialCharacters(const QString& str);

/**
 * replaces newlines ("\n") by &lt;br/>
 * @param str to convert
 */
KDE_EXPORT
QString convertNewlines(const QString& str);
        
/**
 * converts a plain text string to HTML
 *
 * @param plainText a string in plain text.
 */
KDE_EXPORT
QString plainTextToHtml(const QString& plainText);

/**
 * converts a HTML string to plain text
 *
 * @param html string in HTML format
 * @return stripped text
 */
KDE_EXPORT
QString htmlToPlainText(const QString& html);

/**
 * guesses whether a string contains plain text or HTML
 *
 * @param str the string in unknown format
 * @return @c true if the heuristic thinks it's HTML, @c false
 * if thinks it is plain text
 */
KDE_EXPORT
bool isHtml(const QString& str);

/**
 * guesses whether a string contains (HTML) markup or not. This
 * implements not an exact check for valid HTML markup, but a
 * simple (and relatively fast) heuristic.
 *
 * @param str the string that might or might not contain markup
 * @return @c true if the heuristic thinks it contains markup, @c false
 * if thinks it is markup-free plain text
 */
KDE_EXPORT
bool stringContainsMarkup(const QString& str);

/**
 * Ensures HTML formatting for a string.
 * guesses via isHtml() if @c str contains HTML or plain text, and returns
 * plainTextToHtml(str) if it thinks it is plain text, or the unmodified @c str
 * otherwise.
 * 
 * @param str a string with unknown content
 * @return string as HTML (as long as the heuristics work)
 */
KDE_EXPORT
QString normalize(const QString& str);

/**
 * normalizes a string based on feed-wide properties of tag content.
 * It is based on the assumption that all items in a feed encode their 
 * title/description content in the same way (CDATA or not, plain text
 * vs. HTML). isCDATA and containsMarkup are determined once by the feed,
 * and then passed to this method.
 *
 * The returned string contains HTML, with special characters &lt;, >,
 * &, ", and ' escaped, and all other entities resolved.
 * Whitespace is collapsed, relevant whitespace is replaced by respective
 * HTML tags (&lt;br/>).
 * 
 * @param str a string 
 * @param isCDATA whether the feed uses CDATA for the tag @c str was read from
 * @param containsMarkup whether the feed uses HTML markup in the
 *        tag @c str was read from.
 * @return string as HTML (as long as the heuristics work)
 */
KDE_EXPORT
QString normalize(const QString& str, bool isCDATA, bool containsMarkup);

} // namespace LibSyndication

#endif // LIBSYNDICATION_TOOLS_H
