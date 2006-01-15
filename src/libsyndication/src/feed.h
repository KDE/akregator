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

#ifndef LIBSYNDICATION_FEED_H
#define LIBSYNDICATION_FEED_H

#include <ksharedptr.h>

template <class T> class QList;
class QString;

namespace LibSyndication {

class AbstractDocument;
typedef KSharedPtr<AbstractDocument> AbstractDocumentPtr;
class Feed;
typedef KSharedPtr<Feed> FeedPtr;
class Item;
typedef KSharedPtr<Item> ItemPtr;

class Feed : public KShared
{
    public:
        
        virtual ~Feed() {}
        
        virtual AbstractDocumentPtr document() const = 0;
        
        virtual QList<ItemPtr> items() const = 0;
        
        virtual QString title() const = 0;
        
        virtual QString link() const = 0;
        
        virtual QString description() const = 0;
        
        virtual QString author() const = 0;
        
        virtual QString language() const = 0;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_FEED_H
