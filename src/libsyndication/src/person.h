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

#ifndef LIBSYNDICATION_PERSON_H
#define LIBSYNDICATION_PERSON_H

#include "sharedptr.h"

#include <kdepimmacros.h>

namespace LibSyndication {

class Person;
typedef SharedPtr<Person> PersonPtr;

/**
 * Person holds information about a person, such as the author of the whole feed
 * or single items. Depending on the feed format, different information is 
 * available.
 * While according to the RSS2 spec, RSS2 author elements must contain only an
 * e-mail address, Atom requires the person's name and the e-mail address is
 * optional. Also, in reality, feeds often contain other information than what
 * is specified in the specs. LibSyndication tries to find out what author 
 * information is contained and maps it to this representation.
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT Person
{
    public:

        /** 
         * destructor
         */
        virtual ~Person();
        
        /**
         * returns whether this object is a null category
         */
        virtual bool isNull() const = 0;
        
        /**
         * the name of the person (optional)
         * 
         * @return the name of the person as plain text,
         * or a null string if not specified
         */
        virtual QString name() const = 0;
        
        /**
         * a URI associated with the person. (optional)
         * This is usually the URL of the
         * person's homepage. 
         * 
         * @return URI of the person, or a null string if not specified
         */
        virtual QString uri() const = 0;

        /**
         * e-mail address of the person (optional)
         * 
         * @return email address, or a null string if not specified
         */
        virtual QString email() const = 0;

        /**
         * description of the person for debugging purposes.
         * 
         * @return debug string
         */
        virtual QString debugInfo() const;
        
        /**
         * compares two person instances. Persons are equal if and only if
         * their respective name(), uri() and email() are equal.
         * @param other another person instance
         */
        virtual bool operator==(const Person& other) const;
};
    
} // namespace LibSyndication

#endif // LIBSYNDICATION_PERSON_H
