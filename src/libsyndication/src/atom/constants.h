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
#ifndef LIBSYNDICATION_ATOM_CONSTANTS_H
#define LIBSYNDICATION_ATOM_CONSTANTS_H

#include <kdepimmacros.h>

class QString;

namespace LibSyndication {
namespace Atom {

/**
 * some constants used for parsing Atom
 *
 * @author Frank Osterfeld
 */
class KDE_EXPORT Constants
{
    public:
        /**
         * namespace used by Atom 1.0 elements
         */
        static QString atom1Namespace();
        
        /**
         * namespace used by Atom 0.3 elements
         */
        static QString atom0_3Namespace();
};

} // namespace Atom
} // namespace LibSyndication

#endif // LIBSYNDICATION_ATOM_CONSTANTS_H
