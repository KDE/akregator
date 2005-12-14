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
#include "tools.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

class Image::ImagePrivate : public KShared
{
    public:

    QString url;
    QString title;
    QString link;
    int width;
    int height;
    QString description;

    bool operator==(const ImagePrivate& other) const
    {
        return (url == other.url &&
                title == other.title &&
                link == other.link &&
                width == other.width &&
                height == other.height &&
                description == other.description);
    }
};

Image* Image::m_null = 0;
static KStaticDeleter<Image> imagesd;

const Image& Image::null()
{
    if (m_null == 0)
        imagesd.setObject(m_null, new Image);
    return *m_null;
}

Image Image::fromXML(const QDomElement& e)
{
    QString url = Tools::extractElementText(e, QString::fromLatin1("url"));
    QString title = Tools::extractElementText(e, QString::fromLatin1("title"));
    QString link = Tools::extractElementText(e, QString::fromLatin1("link"));
    QString description = Tools::extractElementText(e, QString::fromLatin1("description"));

    QString text;
    bool ok;
    int c;

    text = Tools::extractElementText(e, QString::fromLatin1("width"));
    c = text.toInt(&ok);
    int width = ok ? c : 88; // set to default if not parsable

    text = Tools::extractElementText(e, QString::fromLatin1("height"));
    c = text.toInt(&ok);
    int height = ok ? c : 31; // set to default if not parsable

    return Image(url, title, link, description, width, height);
}

Image::Image() : d(0)
{
}

Image::Image(const Image& other) : d(0)
{
    *this = other;
}

Image::Image(const QString& url, const QString& title, const QString& link, const QString& description, int width, int height) : d(new ImagePrivate)
{
    d->url = url;
    d->title = title;
    d->link = link;
    d->description = description;
    d->width = width;
    d->height = height;
}

Image::~Image()
{
}

bool Image::isNull() const
{
    return !d;
}

Image& Image::operator=(const Image& other)
{
    d = other.d;
    return *this;
}

bool Image::operator==(const Image& other) const
{
    if (!d || !other.d)
        return d == other.d;

    return *d == *other.d;
}

QString Image::url() const
{
    return d ? d->url : QString::null;
}

QString Image::title() const
{
    return d ? d->title : QString::null;
}

QString Image::link() const
{
    return d ? d->link : QString::null;
}

int Image::width() const
{
    return d ? d->width : -1;
}

int Image::height() const
{
    return d ? d->height : -1;
}

QString Image::description() const
{
    return d ? d->description : QString::null;
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
