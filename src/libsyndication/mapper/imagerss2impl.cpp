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

#include "imagerss2impl.h"

#include <QString>

namespace LibSyndication {

ImageRSS2Impl::ImageRSS2Impl(const LibSyndication::RSS2::Image& image)
    : m_image(image)
{}
        
bool ImageRSS2Impl::isNull() const
{
    return m_image.isNull();
}

QString ImageRSS2Impl::url() const
{
    return m_image.url();
}

QString ImageRSS2Impl::title() const
{
    return m_image.title();
}

QString ImageRSS2Impl::link() const
{
    return m_image.link();
}

QString ImageRSS2Impl::description() const
{
    return m_image.description();
}

uint ImageRSS2Impl::height() const
{
    return m_image.height();
}
 
 uint ImageRSS2Impl::width() const
{
    return m_image.width();
}

} // namespace LibSyndication
