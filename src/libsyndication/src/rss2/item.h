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

    void setTitle(const QString& title);
    QString title() const;

    void setLink(const QString& title);
    QString link() const;

    void setDescription(const QString& description);
    QString description() const;

    /** returns data stored in content:encoded, xhtml:body, xhtml:div */
    QString content() const;
    void setContent(const QString& content);

    void addCategory(const Category& category);
    void setCategories(const QList<Category>& categories);
    void removeCategory(const Category& category);
    QList<Category> categories() const;

    void setComments(const QString& comments);
    QString comments() const;

    void setAuthor(const QString& author);
    QString author() const;

    void setEnclosure(const Enclosure& enclosure);
    Enclosure enclosure() const;

    void setGuid(const QString& guid);
    QString guid() const;

    /** default: true */
    void setGuidIsPermaLink(bool isPermaLink);
    bool guidIsPermaLink() const;

    void setPubDate(const QDateTime& pubDate);
    QDateTime pubDate() const;

    void setSource(const Source& source);
    Source source() const;

    QString debugInfo() const;
    private:

    static Item* m_null;

    class ItemPrivate;
    ItemPrivate* d;
};

} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_ITEM_H
