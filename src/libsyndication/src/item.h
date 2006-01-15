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

#ifndef LIBSYNDICATION_ITEM_H
#define LIBSYNDICATION_ITEM_H

namespace LibSyndication {
    
class Item
{
    public:
        
        virtual ~Item() {}
        
        virtual QString title() const = 0;
        
        virtual QString link() const = 0;
        
        virtual QString description() const = 0;
        
        /**
         * 
         * TODO: specify format (HTML)
         * @return content, or QString::null if not set
         */
        virtual QString content() const = 0;
        
        virtual QString author() const = 0;
        
        virtual QString language() const = 0;
        
        /**
         * returns an ID that identifies the item within its feed. The ID must
         * be unique within its feed. If no ID is provided by the feed source,
         * a hash from title, description and content is returned
         */
        virtual QString id() const = 0;
        
        //Enclosure enclosure() const = 0;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_ITEM_H
