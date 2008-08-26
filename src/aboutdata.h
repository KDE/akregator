/*
    This file is part of Akregator.

    Copyright (C) 2004 Teemu Rytilahti <tpr@d5k.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_ABOUTDATA_H
#define AKREGATOR_ABOUTDATA_H

#include "akregator_export.h"
#include <kaboutdata.h>

#define AKREGATOR_VERSION "1.3.50"

namespace Akregator {
/**
@author Teemu Rytilahti
*/
class AKREGATOR_EXPORT AboutData : public KAboutData
{
public:
    AboutData();
    ~AboutData();
};

} // namespace Akregator

#endif // AKREGATOR_ABOUTDATA_H
