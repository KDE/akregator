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

#ifndef LIBSYNDICATION_CATEGORY_H
#define LIBSYNDICATION_CATEGORY_H

#include <ksharedptr.h>

namespace LibSyndication {

class Category;
typedef KSharedPtr<Category> CategoryPtr;

/**
 *
 * 
 * @author Frank Osterfeld
 */
class Category : public KShared
{
    public:

        /** 
         * destructor
         */
        virtual ~Category() {}
        
        virtual bool isNull() const = 0;
        /**
         * 
         * @return 
         */
        virtual QString term() const = 0;
        
        /**
         * 
         * @return 
         */
        virtual QString scheme() const = 0;
        
        /**
         * 
         * @return 
         */
        virtual QString label() const = 0;
        
        /**
         * 
         * 
         * @return debug string
         */
        virtual QString debugInfo() const;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_CATEGORY_H
