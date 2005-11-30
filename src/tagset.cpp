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

#include "tag.h"
#include "tagset.h"

#include <qdom.h>
#include <QHash>
#include <qstring.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <QList>

namespace Akregator {

class TagSet::TagSetPrivate 
{
    public:
    QHash<QString,Tag> hash;
};

TagSet::TagSet(QObject* parent) : QObject(parent), d(new TagSetPrivate)
{
}

TagSet::~TagSet()
{
    QList<Tag> tags = d->hash.values();
    for (QList<Tag>::Iterator it = tags.begin(); it != tags.end(); ++it)
        (*it).removedFromTagSet(this);
    
    delete d;
    d = 0;
}

void TagSet::insert(const Tag& tag)
{
    if (!d->hash.contains(tag.id()))
    {
        d->hash.insert(tag.id(), tag);
        tag.addedToTagSet(this);
        emit signalTagAdded(tag);
    }
}

void TagSet::remove(const Tag& tag)
{
    if (d->hash.contains(tag.id()))
    {
        d->hash.remove(tag.id());
        tag.removedFromTagSet(this);
        emit signalTagRemoved(tag);
    }
}

bool TagSet::containsID(const QString& id) const
{
    return d->hash.contains(id);
}

bool TagSet::contains(const Tag& tag) const
{
    return d->hash.contains(tag.id());
}

Tag TagSet::findByID(const QString& id) const
{
    return d->hash.contains(id) ? d->hash[id] : Tag();
}

QHash<QString,Tag> TagSet::toHash() const
{
    return d->hash;
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
                QString scheme = e.attribute(QString::fromLatin1("scheme"));
                Tag tag(id, name, scheme);
                
                QString icon = e.attribute(QString::fromLatin1("icon"));
                if (!icon.isEmpty())
                    tag.setIcon(icon);

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

    QList<Tag> list = d->hash.values();
    for (QList<Tag>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {    
    
        QDomElement tn = doc.createElement("tag");
        
        QDomText text = doc.createTextNode((*it).name());
        tn.setAttribute(QString::fromLatin1("id"),(*it).id());
        if (!(*it).scheme().isEmpty())
            tn.setAttribute(QString::fromLatin1("scheme"),(*it).scheme());
        if (!(*it).icon().isNull())
            tn.setAttribute(QString::fromLatin1("icon"),(*it).icon());
        tn.appendChild(text);
        root.appendChild(tn);
    }
    return doc;
}

} // namespace Akregator

#include "tagset.moc"
