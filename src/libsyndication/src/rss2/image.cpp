/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "image.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace RSS2 {

Image::Image() : ElementWrapper()
{
}

Image::Image(const QDomElement& element) : ElementWrapper(element)
{
}

QString Image::url() const
{
    return extractElementText(QString::fromUtf8("url"));
}

QString Image::title() const
{
    return extractElementText(QString::fromUtf8("title"));
    
}

QString Image::link() const
{
    return extractElementText(QString::fromUtf8("link"));
    
}

uint Image::width() const
{
    QString text;
    bool ok;
    uint c;

    text = extractElementText(QString::fromUtf8("width"));
    c = text.toUInt(&ok);
    return ok ? c : 88; // set to default if not parsable
}

uint Image::height() const
{
    QString text;
    bool ok;
    uint c;

    text = extractElementText(QString::fromUtf8("height"));
    c = text.toUInt(&ok);
    return ok ? c : 31; // set to default if not parsable
}

QString Image::description() const
{
    return extractElementText(QString::fromUtf8("description"));
}

QString Image::debugInfo() const
{
    QString info;
    info += "### Image: ###################\n";
    info += "title: #" + title() + "#\n";
    info += "link: #" + link() + "#\n";
    info += "description: #" + description() + "#\n";
    info += "url: #" + url() + "#\n";
    info += "width: #" + QString::number(width()) + "#\n";
    info += "height: #" + QString::number(height()) + "#\n";
    info += "### Image end ################\n";
    return info;
}

} // namespace RSS2
} //namespace LibSyndication
