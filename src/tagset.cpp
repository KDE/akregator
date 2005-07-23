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

#include "tag.h"
#include "tagset.h"

#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

namespace Akregator {

class TagSet::TagSetPrivate 
{
    public:
    QMap<QString,Tag> map;
};

TagSet::TagSet(QObject* parent) : QObject(parent), d(new TagSetPrivate)
{
}

TagSet::~TagSet()
{
    QValueList<Tag> tags = d->map.values();
    for (QValueList<Tag>::Iterator it = tags.begin(); it != tags.end(); ++it)
        (*it).removedFromTagSet(this);
    
    delete d;
    d = 0;
}

void TagSet::insert(const Tag& tag)
{
    if (!d->map.contains(tag.id()))
    {
        d->map.insert(tag.id(), tag);
        tag.addedToTagSet(this);
        emit signalTagAdded(tag);
    }
}

void TagSet::remove(const Tag& tag)
{
    if (d->map.contains(tag.id()))
    {
        d->map.remove(tag.id());
        tag.removedFromTagSet(this);
        emit signalTagRemoved(tag);
    }
}

bool TagSet::containsID(const QString& id) const
{
    return d->map.contains(id);
}

bool TagSet::contains(const Tag& tag) const
{
    return d->map.contains(tag.id());
}

Tag TagSet::findByID(const QString& id) const
{
    return d->map.contains(id) ? d->map[id] : Tag();
}

QMap<QString,Tag> TagSet::toMap() const
{
    return d->map;
}

void TagSet::readFromXML(const QDomDocument& doc)
{
    QDomElement root = doc.documentElement();

    if (root.isNull())
        return;

    QDomNodeList list = root.elementsByTagName(QString::fromLatin1("tag"));

    for (uint i = 0; i < list.length(); ++i)
    {
        QDomElement e = list.item(i).toElement();
        if (!e.isNull())
        {
            if (e.hasAttribute(QString::fromLatin1("id")))
            {
                QString id = e.attribute(QString::fromLatin1("id"));
                QString name = e.text();
                Tag tag(id, name);
                insert(tag);
            }
        }
    }

}
void TagSet::tagUpdated(const Tag& tag)
{
    emit signalTagUpdated(tag);
}
        
QDomDocument TagSet::toXML() const
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );

    QDomElement root = doc.createElement("tagSet");
    root.setAttribute( "version", "0.1" );
    doc.appendChild(root);

    QValueList<Tag> list = d->map.values();
    for (QValueList<Tag>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {    
    
        QDomElement tn = doc.createElement("tag");
        
        QDomText text = doc.createTextNode((*it).name());
        tn.setAttribute(QString::fromLatin1("id"),(*it).id());
        tn.appendChild(text);
        root.appendChild(tn);
    }
    return doc;
}

} // namespace Akregator

#include "tagset.moc"
