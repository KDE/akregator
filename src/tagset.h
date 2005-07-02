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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_TAGSET_H
#define AKREGATOR_TAGSET_H

#include <qobject.h>

class QDomDocument;
template <class K,class T> class QMap;
class QString;
class QStringList;

namespace Akregator {

class Tag;

/** \brief represents a set of tags (@see Tag)
    In an application, there is usually one central tag set that is used.
    The tag set is not necessarily managed by the application itself, it might also be part of 
    a desktop-wide framework (like Tenor) managing a common tag set for all applications 
    
    @author Frank Osterfeld
*/

class TagSet : public QObject
{
    friend class Tag;
    Q_OBJECT
    public:

        TagSet(QObject* parent=0);
        virtual ~TagSet();

        /** adds a tag to the tag set. The tag set will emit @see signalTagAdded */
        void insert(const Tag& tag);

        /** removes a tag from the tag set. The tag set will emit @see signalTagRemoved */
        void remove(const Tag& tag);

        /** returns the tag set as map ((id, Tag) pairs) */
        QMap<QString,Tag> toMap() const;

        /** returns @c true if this set contains @c tag */
        bool contains(const Tag& tag) const;

        /** returns the tag with the given ID if the tag is element of the set, or a null tag if not */
        Tag findByID(const QString& id) const;

        /** reads tag set from XML
            see @see toXML() for an explanation of the format */
        void readFromXML(const QDomDocument& doc);

        /** returns an XML representation of the tag set. 
            The root element is @c <tagSet>, a tag  ("someID", "someName") is represented as
            \code <tag id="someID">someName</tag> \endcode
            Full example: 
            \code
            <?xml version="1.0" encoding="UTF-8"?>
            <tagSet version="0.1" >
            <tag id="http://akregator.sf.net/tags/Interesting" >Interesting</tag>
            <tag id="AFs3SdaD" >Pretty boring</tag>
            </tagSet>
            \endcode
         */
        QDomDocument toXML() const;

    signals:
        /** emitted when a tag was added to this tag set */
        void signalTagAdded(const Tag&);
        /** emitted when a tag was removed from this set */
        void signalTagRemoved(const Tag&);
        /** emitted when a tag in this set was changed (e.g. renamed) */
        void signalTagUpdated(const Tag&);

    protected:
        /** called by the tag (Tag is friend class) after a change */
        void tagUpdated(const Tag& tag);
        
    private:
        class TagSetPrivate;
        TagSetPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_TAGSET_H
