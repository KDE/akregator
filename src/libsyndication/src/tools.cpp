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

#include "tools.h"

#include <kcharsets.h>
#include <kcodecs.h> 
#include <krfcdate.h>

#include <QByteArray>
#include <QRegExp>
#include <QString>

namespace LibSyndication {

KMD5 md5Machine;

unsigned int calcHash(const QString& str)
{
    return calcHash(str.utf8());
}

unsigned int calcHash(const QByteArray& array)
{
    if (array.isEmpty())
    {
        return 0;
    }
    else
    {
        const char* s = array.data();
        unsigned int hash = 5381;
        int c;
        while ( ( c = *s++ ) ) hash = ((hash << 5) + hash) + c; // hash*33 + c
        return hash;
    }
}

time_t parseISODate(const QString& str)
{
    return KRFCDate::parseDateISO8601(str);
}

time_t parseRFCDate(const QString& str)
{
    return KRFCDate::parseDate(str);
}

QString calcMD5Sum(const QString& str)
{
    md5Machine.reset();
    md5Machine.update(str.utf8());
    return QString(md5Machine.hexDigest().data());
}

QString plainTextToHtml(const QString& plainText)
{
    QString str(plainText);
    str.replace("&", "&amp;");
    str.replace("\"", "&quot;");
    str.replace("<", "&lt;");
    str.replace(">", "&gt;");
    str.replace("\n", "<br/>");
    return str;
}

QString htmlToPlainText(const QString& html)
{
    QString str(html);
    //TODO: preserve some formatting, such as line breaks
    str.replace(QRegExp("<[^>]*>"), ""); // remove tags
    str = KCharsets::resolveEntities(str);
    str = str.simplified();
    
    return str;
}

bool isHtml(const QString& str)
{
    if (str.contains("&amp;") || str.contains("&quot;"))
        return true;
    
    int ltc = str.count('<');
    if (ltc == 0 || ltc != str.count('>'))
        return false;

    if (str.contains(QRegExp("<[a-zA-Z]+/?>")))
        return true;
        
    return false; 
}

QString htmlize(const QString& str)
{
    return isHtml(str) ? str : plainTextToHtml(str);
}

} // namespace LibSyndication


