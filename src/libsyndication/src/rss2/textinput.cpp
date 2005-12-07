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

#include "textinput.h"
#include "tools.h"

#include <qdom.h>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

class TextInput::TextInputPrivate : public Shared
{
    public:

    bool isNull;
    QString title;
    QString name;
    QString description;
    QString link;

    bool operator==(const TextInputPrivate& other) const
    {
        return (isNull && other.isNull) || (
                title == other.title &&
                name == other.name &&
                description == other.description &&
                link == other.link);
    }

    static TextInputPrivate* copyOnWrite(TextInputPrivate* ep)
    {
        if (ep->count > 1)
        {
            ep->deref();
            ep = new TextInputPrivate(*ep);
        }
        return ep;
    }
};

TextInput* TextInput::m_null = 0;
static KStaticDeleter<TextInput> textinputsd;

const TextInput& TextInput::null()
{
    if (m_null == 0)
        textinputsd.setObject(m_null, new TextInput);
    return *m_null;
}

TextInput TextInput::fromXML(const QDomElement& e)
{
    TextInput obj;
    obj.setName(Tools::extractElementText(e, QString::fromLatin1("name") ));
    obj.setTitle(Tools::extractElementText(e, QString::fromLatin1("title") ));
    obj.setLink(Tools::extractElementText(e, QString::fromLatin1("link") ));
    obj.setDescription(Tools::extractElementText(e, QString::fromLatin1("description") ));
    return obj;
}

TextInput::TextInput() : d(new TextInputPrivate)
{
    d->isNull = true;
}

TextInput::TextInput(const TextInput& other) : d(0)
{
    *this = other;
}

TextInput::~TextInput()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

TextInput& TextInput::operator=(const TextInput& other)
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

bool TextInput::operator==(const TextInput& other) const
{
    return *d == *other.d;
}

bool TextInput::isNull() const
{
    return d->isNull;
}

void TextInput::setTitle(const QString& title)
{
    d = TextInputPrivate::copyOnWrite(d);
    d->isNull = false;
    d->title = title;
}

QString TextInput::title() const
{
    return !d->isNull ? d->title : QString::null;
}

void TextInput::setName(const QString& name)
{
    d = TextInputPrivate::copyOnWrite(d);
    d->isNull = false;
    d->name = name;
}

QString TextInput::name() const
{
    return !d->isNull ? d->name : QString::null;
}

void TextInput::setDescription(const QString& description)
{
    d = TextInputPrivate::copyOnWrite(d);
    d->isNull = false;
    d->description = description;
}

QString TextInput::description() const
{
    return !d->isNull ? d->description : QString::null;
}

void TextInput::setLink(const QString& link)
{
    d = TextInputPrivate::copyOnWrite(d);
    d->isNull = false;
    d->link = link;
}

QString TextInput::link() const
{
    return !d->isNull ? d->link : QString::null;
}

QString TextInput::debugInfo() const
{
    QString info;
    info += "### TextInput: ###################\n";
    info += "title: #" + title() + "#\n";
    info += "link: #" + link() + "#\n";
    info += "description: #" + description() + "#\n";
    info += "name: #" + name() + "#\n";
    info += "### TextInput end ################\n";
    return info;
}

} // namespace RSS2
} // namespace LibSyndication
