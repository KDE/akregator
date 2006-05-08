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

#include <QString>
#include <kdepim_export.h>

namespace Akregator {

class TagSet;

/** represents a tag. A tag has a required identifier and optional @c scheme and @c name attributes
 - The identifier is any string and must be unique in the tag set
 - @c name is the human-readible name of the tag. This is the string used in the GUI
 - The optional attribute @c scheme is a classification scheme the tag belongs to

Examples:
user-generated tag with some magic in the id to make it (probably) unique: id: ACB4C7D5FFFriends name: Friends
mapped from a blog category: id: http://ablog.org/blog#Funny name: Funny
tag from some ontology: id: http://foo/ont/AnimalTaxonomy/mammals, name: Mammals, scheme: http://foo/ont/AnimalTaxonomy
*/
class AKREGATOR_EXPORT Tag
{
    friend class TagSet;
    
    public:

    /** creates a tag with given id, name and scheme. If name is QString::null, the id is used as name. If id is QString::null, the object is considered as NULL object (see isNull())*/
    Tag(const QString& id, const QString& name=QString::null, const QString& scheme=QString::null);

    /** creates a null tag (isNull() is @c true) */
    Tag();
   
    Tag(const Tag& other);

    /** creates a tag from a Atom-1.0-like (term, scheme, label) category.

      @c term is a string that identifies the tag, Examples are: "General", "KDE", "Personal"
      @c scheme (optional) classification scheme the term belongs to
      @c label/name the (optinal) human-readable name of the tag, synonymous to @c name
      
      Example:
      
      Create

      <category term="foo" scheme="http://del.icio.us/tag"  label="Del.icio.us tag for foo"/>

      using Tag::fromCategory("foo", "http://del.icio.us/tag", "Del.icio.us tag for foo")
      
      The @c id is built using 'scheme + "/" + term': The example gets id = "http://del.icio.us/tag/foo"
    */
    static Tag fromCategory(const QString& term, const QString& scheme=QString::null, const QString& name=QString::null);

    virtual ~Tag();

    /** returns whether this is a null object (equal to id().isNull())*/

    bool isNull() const;
    
    /** tag identifier, used as key throughout the app and archive. Must be unique in the tag set. Can be an arbitrary unicode string, an URI etc. */
    QString id() const;

    /** user-visible name of the tag */
    QString name() const;

    /** (optional) classfication scheme this tag belongs to */
    QString scheme() const;

    void setName(const QString& name);

    QString icon() const;
    void setIcon(const QString& icon);

    Tag& operator=(const Tag& other);

    /** compares tags by name. If names are equal, id's are compared. 
        a < b iff a.name < b.name || (a.name == b.name && a.id < b.id) */
    bool operator<(const Tag& other) const;

    /** tags are equal when their id's are equal, name is ignored */
    bool operator==(const Tag& other) const;


    protected:
    /** called by TagSet */
    void addedToTagSet(TagSet* tagSet) const;

/** called by TagSet */
    void removedFromTagSet(TagSet* tagSet) const;
    private:

    class TagPrivate;
    TagPrivate* d;
};


} // namespace Akregator

#endif // AKREGATOR_TAG_H
