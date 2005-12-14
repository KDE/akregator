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

#include "textinput.h"
#include "tools.h"

#include <qdom.h>
#include <QString>

#include <kstaticdeleter.h>

namespace LibSyndication {
namespace RSS2 {

class TextInput::TextInputPrivate : public KShared
{
    public:

    QString title;
    QString name;
    QString description;
    QString link;

    bool operator==(const TextInputPrivate& other) const
    {
        return (
                title == other.title &&
                name == other.name &&
                description == other.description &&
                link == other.link);
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
    QString name = Tools::extractElementText(e, QString::fromLatin1("name") );
    QString title = Tools::extractElementText(e, QString::fromLatin1("title") );
    QString link = Tools::extractElementText(e, QString::fromLatin1("link") );
    QString description = Tools::extractElementText(e, QString::fromLatin1("description") );
    return TextInput(title, description, link, name);
}

TextInput::TextInput() : d(0)
{
}

TextInput::TextInput(const QString& title, const QString& description, const QString&  link, const QString& name) : d(new TextInputPrivate)
{
    d->title = title;
    d->description = description;
    d->link = link;
    d->name = name; 
}

TextInput::TextInput(const TextInput& other) : d(0)
{
    *this = other;
}

TextInput::~TextInput()
{
}

TextInput& TextInput::operator=(const TextInput& other)
{
    d = other.d;
    return *this;
}

bool TextInput::operator==(const TextInput& other) const
{
    if (!d || !other.d)
        return d == other.d;
    return *d == *other.d;
}

bool TextInput::isNull() const
{
    return !d;
}

QString TextInput::title() const
{
    return d ? d->title : QString::null;
}

QString TextInput::name() const
{
    return d ? d->name : QString::null;
}

QString TextInput::description() const
{
    return d ? d->description : QString::null;
}

QString TextInput::link() const
{
    return d ? d->link : QString::null;
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
