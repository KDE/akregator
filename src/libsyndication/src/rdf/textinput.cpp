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

#include "rssvocab.h"
#include "property.h"
#include "resource.h"
#include "statement.h"
#include "textinput.h"

#include <QString>

namespace LibSyndication {
namespace RDF {

TextInput::TextInput() : ResourceWrapper()
{
}

TextInput::TextInput(ResourcePtr resource) : ResourceWrapper(resource)
{
}

TextInput::~TextInput()
{
}

QString TextInput::title() const
{
    return resource()->property(RSSVocab::self()->title())->asString();
}

QString TextInput::description() const
{
    return resource()->property(RSSVocab::self()->description())->asString();
}

QString TextInput::link() const
{
    return resource()->property(RSSVocab::self()->link())->asString();
}

QString TextInput::name() const
{
    return resource()->property(RSSVocab::self()->name())->asString();
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


} // namespace RDF
} // namespace LibSyndication
