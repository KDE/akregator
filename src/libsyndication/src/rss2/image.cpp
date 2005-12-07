/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "../shared.h"

#include "image.h"
#include "tools.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

class Image::ImagePrivate : public LibSyndication::Shared
{
    public:

    bool isNull;
    QString url;
    QString title;
    QString link;
    int width;
    int height;
    QString description;

    bool operator==(const ImagePrivate& other) const
    {
        return (isNull && other.isNull) || (
                url == other.url &&
                title == other.title &&
                link == other.link &&
                width == other.width &&
                height == other.height &&
                description == other.description);
    }

    static ImagePrivate* copyOnWrite(ImagePrivate* ep)
    {
        if (ep->count > 1)
        {
            ep->deref();
            ep = new ImagePrivate(*ep);
        }
        return ep;
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
    Image obj;
    obj.setURL(Tools::extractElementText(e, QString::fromLatin1("url") ));
    obj.setTitle(Tools::extractElementText(e, QString::fromLatin1("title") ));
    obj.setLink(Tools::extractElementText(e, QString::fromLatin1("link") ));
    obj.setDescription(Tools::extractElementText(e, QString::fromLatin1("description") ));

    QString text;
    bool ok;
    int c;

    text = Tools::extractElementText(e, QString::fromLatin1("width"));
    c = text.toInt(&ok);
    obj.setWidth(ok ? c : 88); // set to default if not parsable

    text = Tools::extractElementText(e, QString::fromLatin1("height"));
    c = text.toInt(&ok);
    obj.setHeight(ok ? c : 13); // set to default if not parsable

    return obj;
}

Image::Image() : d(new ImagePrivate)
{
    d->isNull = true;
    d->width = -1;
    d->height = -1;
}

Image::Image(const Image& other) : d(0)
{
    *this = other;
}

Image::Image(const QString& url, const QString& title, const QString& link) : d(new ImagePrivate)
{
    d->isNull = false;
    d->url = url;
    d->title = title;
    d->link = link;
    d->width = -1;
    d->height = -1;
}

Image::~Image()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

bool Image::isNull() const
{
    return d->isNull;
}

Image& Image::operator=(const Image& other)
{
    if (d != other.d)
    {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}

bool Image::operator==(const Image& other) const
{
    return *d == *other.d;
}

void Image::setURL(const QString& url)
{
    d = ImagePrivate::copyOnWrite(d);
    d->isNull = false;
    d->url = url;
}

QString Image::url() const
{
    return !d->isNull ? d->url : QString::null;
}

void Image::setTitle(const QString& title)
{
    d = ImagePrivate::copyOnWrite(d);
    d->isNull = false;
    d->title = title;
}

QString Image::title() const
{
    return !d->isNull ? d->title : QString::null;
}

void Image::setLink(const QString& link)
{
    d = ImagePrivate::copyOnWrite(d);
    d->isNull = false;
    d->link = link;
}

QString Image::link() const
{
    return !d->isNull ? d->link : QString::null;
}

void Image::setWidth(int width)
{
    d = ImagePrivate::copyOnWrite(d);
    d->isNull = false;
    d->width = width;
}

int Image::width() const
{
    return !d->isNull ? d->width : -1;
}

void Image::setHeight(int height)
{
    d = ImagePrivate::copyOnWrite(d);
    d->isNull = false;
    d->height = height;
}

int Image::height() const
{
    return !d->isNull ? d->height : -1;
}

void Image::setDescription(const QString& description)
{
    d = ImagePrivate::copyOnWrite(d);
    d->isNull = false;
    d->description = description;
}

QString Image::description() const
{
    return !d->isNull ? d->description : QString::null;
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

} // namespace RSS
} //namespace LibSyndication
