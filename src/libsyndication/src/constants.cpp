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

#include "constants.h"

#include <QString>

namespace LibSyndication {

QString xmlNamespace()
{
    return QString::fromUtf8("http://www.w3.org/XML/1998/namespace");
}

QString xhtmlNamespace()
{
    return QString::fromUtf8("http://www.w3.org/1999/xhtml");
}

QString contentNameSpace()
{
    return QString::fromUtf8("http://purl.org/rss/1.0/modules/content/");
}

QString dublinCoreNamespace()
{
    return QString::fromUtf8("http://purl.org/dc/elements/1.1/");
}

QString itunesNamespace()
{
    return QString::fromUtf8("http://www.itunes.com/dtds/podcast-1.0.dtd");
}

QString commentApiNamespace()
{
    return QString::fromUtf8("http://wellformedweb.org/CommentAPI/");
}

QString slashNamespace()
{
    return QString::fromUtf8("http://purl.org/rss/1.0/modules/slash/");
}

} // namespace LibSyndication
