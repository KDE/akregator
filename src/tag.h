/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef AKREGATOR_TAG_H
#define AKREGATOR_TAG_H

#include <qstring.h>

namespace Akregator {

class TagSet;
/** represents a tag. A tag has an identifier and a name. An identifier is a unique key, where the name is the user-visible name
The id MUST be unique, the name SHOULD.
Examples:
user-generated tag with some magic in the id to make it (probably) unique: id: ACB4C7D5FFFriends name: Friends
mapped from a blog category: id: http://ablog.org/blog#Funny name: Funny
tag from some ontology: id: http://foo/ont/Animals name: Animals
*/
class Tag
{
    friend class TagSet;
    
    public:

    /** creates a tag with given id and name. If name is QString::null, the id is used as name. If id is QString::null, the object is considered as NULL object (see isNull())*/
    Tag(const QString& id, const QString& name=QString::null);

    /** creates a null tag (isNull() is @c true) */
    Tag();
    
    Tag(const Tag& other);

    virtual ~Tag();

    /** returns whether this is a null object (equal to id().isNull())*/

    bool isNull() const;
    
    /** tag identifier, used as key throughout the app and archive. Must be unique in the tag set. Can be an arbitrary unicode string, an URI etc. */
    QString id() const;

    /** user-visible name of the tag */
    QString name() const;

    void setName(const QString& name);

    Tag& operator=(const Tag& other);

    /** tags are equal when their id's are equal, name is ignored */
    bool operator==(const Tag& other) const;


    protected:

    void addedToTagSet(TagSet* tagSet) const;
    void removedFromTagSet(TagSet* tagSet) const;
    private:

    class TagPrivate;
    TagPrivate* d;
};


} // namespace Akregator

#endif // AKREGATOR_TAG_H
