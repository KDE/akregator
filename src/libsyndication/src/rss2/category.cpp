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

#include "category.h"
#include "tools.h"

#include <QString>

class QString;

namespace LibSyndication {
namespace RSS2 {

Category::Category() : ElementWrapper()
{
}

Category::Category(const QDomElement& element) : ElementWrapper(element)
{
}
QString Category::category() const
{
    return text().simplified();
}

QString Category::domain() const
{
    return attribute(QString::fromUtf8("domain"));
}

QString Category::debugInfo() const
{
    QString info;
    info += "### Category: ###################\n";
    if (!category().isNull())
        info += "category: #" + category() + "#\n";
    if (!domain().isNull())
        info += "domain: #" + domain() + "#\n";
    info += "### Category end ################\n";
    return info;
}

} // namespace RSS2
} // namespace LibSyndication
