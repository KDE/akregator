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
time_t parseISODate(const QString& str);

/**
 * parses a date string as defined in RFC 822.
 * (Sat, 07 Sep 2002 00:00:01 GMT)
 * 
 * @param str a string in RFC 822 format
 * @return parsed date in seconds since epoch, 0 if no date could
 * be parsed from the string.
 */
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
QString dateTimeToString(time_t date);

/**
 * converts a plain text string to HTML
 *
 * @param plainText a string in plain text.
 */
QString plainTextToHtml(const QString& plainText);

/**
 * converts a HTML string to plain text
 *
 * @param html
 */
QString htmlToPlainText(const QString& html);

/**
 * guesses whether a string contains plain text or HTML
 *
 * @param str
 * @return
 */
bool isHtml(const QString& str);

/**
 * 
 * @param str
 * @return 
 */
QString htmlize(const QString& str);

} // namespace LibSyndication

#endif // LIBSYNDICATION_TOOLS_H
