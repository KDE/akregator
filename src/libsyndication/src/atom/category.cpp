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

#include "category.h"
#include "constants.h"

#include <QDomElement>
#include <QString>

namespace LibSyndication {
namespace Atom {

Category::Category() : ElementWrapper()
{
}

Category::Category(const QDomElement& element) : ElementWrapper(element)
{
}

QString Category::term() const
{
    return element().attribute(QString::fromUtf8("term"));
}

QString Category::scheme() const
{
    return completeURI(element().attribute(QString::fromUtf8("scheme")));
}

QString Category::label() const
{
    return element().attribute(QString::fromUtf8("label"));
}

QString Category::debugInfo() const
{
    QString info;
    info += "### Category: ###################\n";
    info += "term: #" + term() + "#\n";
    if (!scheme().isEmpty())
        info += "scheme: #" + scheme() + "#\n";
    if (!label().isEmpty())
        info += "label: #" + label() + "#\n";
    info += "### Category end ################\n";

    return info;
}

} // namespace Atom
} //namespace LibSyndication
