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

#include <QString>

#include "image.h"

namespace LibSyndication {

QString Image::debugInfo() const
{
    QString info;
    info += "# Image begin #####################\n";

    QString durl = url();
    
    if (!durl.isNull())
    {
        info += "url: #" + durl + "#\n";
    }
    
    QString dtitle = title();
    
    if (!dtitle.isNull())
    {
        info += "title: #" + dtitle + "#\n";
    }
    
    QString dlink = link();
    
    if (!dlink.isNull())
    {
        info += "link: #" + dlink + "#\n";
    }
    
    QString ddescription = description();
    
    if (!ddescription.isNull())
    {
        info += "description: #" + ddescription + "#\n";
    }
    
    int dheight = height();
    if (dheight > 0)
        info += "height: #" + QString::number(height()) + "#\n";

    int dwidth = width();
    if (dwidth > 0)
        info += "width: #" + QString::number(width()) + "#\n";
    info += "# Image end #######################\n";
    
    return info;
}

} // namespace LibSyndication
