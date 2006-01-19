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

#ifndef LIBSYNDICATION_MAPPER_ITEMRSS2IMPL_H
#define LIBSYNDICATION_MAPPER_ITEMRSS2IMPL_H

#include "../rss2/item.h"
#include "../item.h"

namespace LibSyndication {
namespace Mapper {

class ItemRSS2Impl;
typedef KSharedPtr<ItemRSS2Impl> ItemRSS2ImplPtr;

class ItemRSS2Impl : public LibSyndication::Item
{
    public:
        
        ItemRSS2Impl(const LibSyndication::RSS2::Item& item);
        
        QString title() const;
        
        QString link() const;
        
        QString description() const;
        
        QString content() const;
        
        QString author() const;
        
        QString language() const;
        
        QString id() const;
        
        time_t datePublished() const;
        
        time_t dateUpdated() const;
        
        QList<EnclosurePtr> enclosures() const;
        
        QList<CategoryPtr> categories() const;
        
    private:
        
        LibSyndication::RSS2::Item m_item;
};

} // namespace Mapper
} // namespace LibSyndication

#endif // LIBSYNDICATION_MAPPER_ITEMRSS2IMPL_H
