/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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
    static QString stripTags(QString str);

    /** taken from some website... -fo
    * djb2
    * This algorithm was first reported by Dan Bernstein
    * many years ago in comp.lang.c
    */

    static uint calcHash(const QString &str);

    static QString convertHtmlTags(const QString &title);
};
} // namespace Akregator

#endif // AKREGATOR_UTILS_H
