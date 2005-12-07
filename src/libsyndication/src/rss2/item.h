/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#ifndef LIBSYNDICATION_RSS2_ITEM_H
#define LIBSYNDICATION_RSS2_ITEM_H

#include <ksharedptr.h>

class QDateTime;
class QDomDocument;
class QDomElement;
class QString;
template <class T> class QList;

namespace LibSyndication {
namespace RSS2
{

class Category;
class Enclosure;
class Source;

class Item
{
    public:

    static const Item& null();

    static Item fromXML(const QDomElement& e);

    Item();
    Item(const Item& other);
    virtual ~Item();

    Item& operator=(const Item& other);
    bool operator==(const Item& other) const;

    bool isNull() const;

    QString title() const;

    QString link() const;

    QString description() const;

    /** returns data stored in content:encoded, xhtml:body, xhtml:div */
    QString content() const;

    QList<Category> categories() const;

    QString comments() const;

    QString author() const;

    Enclosure enclosure() const;

    QString guid() const;

    /** default: true */
    bool guidIsPermaLink() const;

    QDateTime pubDate() const;

    Source source() const;

    QString debugInfo() const;

    private:

    Item(const QString& title, const QString& link, const QString& description,
         const QString& content, const QList<Category>& categories,
         const QString& comments, const QString& author, 
         const Enclosure& enclosure, const QString& guid, bool guidIsPermaLink,
         const QDateTime& pubDate, const Source& source);

    static Item* m_null;

    class ItemPrivate;
    KSharedPtr<ItemPrivate> d;
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_ITEM_H
