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

#ifndef LIBSYNDICATION_MAPPER_CATEGORYRSS2IMPL_H
#define LIBSYNDICATION_MAPPER_CATEGORYRSS2IMPL_H

#include "../category.h"
#include "../rss2/category.h"

#include <ksharedptr.h>

namespace LibSyndication {
namespace Mapper {
    
class CategoryRSS2Impl;
typedef KSharedPtr<CategoryRSS2Impl> CategoryRSS2ImplPtr;

/**
 *
 * @internal
 * @author Frank Osterfeld
 */
class CategoryRSS2Impl : public LibSyndication::Category
{
    public:

        CategoryRSS2Impl(const LibSyndication::RSS2::Category& category);
        
        bool isNull() const;
        
        QString term() const;
        
        QString label() const;
        
        QString scheme() const;
        
    private:
        LibSyndication::RSS2::Category m_category;
};
    
} // namespace Mapper
} // namespace LibSyndication

#endif // LIBSYNDICATION_MAPPER_CATEGORYRSS2IMPL_H
