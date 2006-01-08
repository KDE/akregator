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

#include "image.h"
#include "rssvocab.h"
#include "statement.h"

#include <QString>

namespace LibSyndication {
namespace RDF {
    
Image::Image() : Resource()
{
}

Image::Image(const Resource& resource) : Resource(resource)
{
}

Image::Image(const QString& uri, const Model& model) : Resource (uri, model)
{
}

Image::~Image()
{
}

QString Image::title() const
{
    return property(RSSVocab::self()->title()).asString();
}
    
QString Image::description() const
{
    return property(RSSVocab::self()->description()).asString();
}
    
QString Image::url() const
{
    return property(RSSVocab::self()->url()).asString();
}
    
QString Image::debugInfo() const
{
    QString info;
    info += "### Image: ###################\n";
    info += "title: #" + title() + "#\n";
    info += "description: #" + description() + "#\n";
    info += "url: #" + url() + "#\n";
    info += "### Image end ################\n";
    return info;
}

    
} // namespace RDF
} // namespace LibSyndication
