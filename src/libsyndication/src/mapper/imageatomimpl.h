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

#ifndef LIBSYNDICATION_MAPPER_IMAGEATOMIMPL_H
#define LIBSYNDICATION_MAPPER_IMAGEATOMIMPL_H

#include "../image.h"

#include <QString>

#include <ksharedptr.h>

namespace LibSyndication {
    
class ImageAtomImpl;
typedef KSharedPtr<ImageAtomImpl> ImageAtomImplPtr;

/**
 *
 * @internal
 * @author Frank Osterfeld
 */
class ImageAtomImpl : public LibSyndication::Image
{
    public:
        
        bool isNull() const { return true; }
                
        QString url() const { return QString::null; }
        
        QString title() const { return QString::null; }
        
        QString link() const { return QString::null; }
        
        QString description() const { return QString::null; }
        
        uint width() const { return 0; }
        
        uint height() const { return 0; }
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_MAPPER_IMAGEATOMIMPL_H
