/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_UTILS_H
#define AKREGATOR_UTILS_H

#include "akregator_export.h"
#include <QString>
typedef unsigned int uint;

namespace Akregator {

class AKREGATOR_EXPORT Utils 
{
    public:
    /** removes HTML/XML tags (everything between &lt; and &gt;) from a string.  "<p><strong>foo</strong> bar</p>" becomes "foo bar" */
    static QString stripTags(const QString& str);

    /** taken from some website... -fo
    * djb2
    * This algorithm was first reported by Dan Bernstein
    * many years ago in comp.lang.c
    */

    static uint calcHash(const QString& str);
    
    /**
     * returns a file name for a URL, with chars like "/" ":"
     * replaced by "_". Too long URLs (>255 chars) are shortened and
     * appended with a hash value.
     * 
     */
    static QString fileNameForUrl(const QString& url);

    static QString directionOf(const QString &str);
};

} // namespace Akregator

#endif // AKREGATOR_UTILS_H
