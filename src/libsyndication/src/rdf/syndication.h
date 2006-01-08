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


#ifndef LIBSYNDICATION_RDF_SYNDICATION_H
#define LIBSYNDICATION_RDF_SYNDICATION_H

#include <ksharedptr.h>

#include <ctime>

class QString;

namespace LibSyndication {

class Resource;

namespace RDF {

class Resource;

class Syndication
{
    public:
        
        enum Period { hourly, daily, weekly, monthly, yearly };
                
        Syndication();
        Syndication(const Syndication& other);
        Syndication(const Resource& resource);
        virtual ~Syndication();
                
        Syndication& operator=(const Syndication& other);
        bool operator==(const Syndication& other) const;
                
        Period updatePeriod() const;
        
        int updateFrequency() const;
        
        time_t updateBase() const;
        
        QString debugInfo() const;
        
    private:
        
        class SyndicationPrivate;
        KSharedPtr<SyndicationPrivate> d;
};

} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_CONTENT_H
