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

#ifndef LIBSYNDICATION_RSS2_DOCUMENT_H
#define LIBSYNDICATION_RSS2_DOCUMENT_H

#include "../document.h"

class QDateTime;
class QDomDocument;
class QDomElement;
class QString;

template <class T> class QList;

namespace LibSyndication {
namespace RSS2
{

    class Category;
    class Cloud;
    class Image;
    class Item;
    class TextInput;

    class Document : public LibSyndication::Document
    {
        public:

        static const Document& null();

        static Document fromXML(const QDomDocument& document);
        // TODO: change this to QDomDocument toXML() const;
        QDomElement toXML(QDomDocument document) const;

        Document();
        Document(const Document& other);
        virtual ~Document();

        Document& operator=(const Document& other);
        bool operator==(const Document& other) const;

        bool isNull() const;

	virtual bool accept(DocumentVisitor* visitor);

        void setTitle(const QString& title);
        QString title() const;

        void setLink(const QString& link);
        QString link() const;

        void setDescription(const QString& description);
        QString description() const;

        void setLanguage(const QString& language);
        QString language() const;

        void setCopyright(const QString& copyright);
        QString copyright() const;

        void setManagingEditor(const QString& managingEditor);
        QString managingEditor() const;

        void setWebMaster(const QString& webMaster);
        QString webMaster() const;

        void setPubDate(const QDateTime& pubDate);
        QDateTime pubDate() const;

        void setLastBuildDate(const QDateTime& lastBuildDate);
        QDateTime lastBuildDate() const;

        void addCategory(const Category& category);
        void setCategories(const QList<Category>& categories);
        void removeCategory(const Category& category);
        QList<Category> categories() const;

        void setGenerator(const QString& generator);
        QString generator() const;

        void setDocs(const QString& docs);
        QString docs() const;

        void setCloud(const Cloud& cloud);
        Cloud cloud() const;

        void setTtl(int ttl);
        int ttl() const;

        void setImage(const Image& image);
        Image image() const;

        void setTextInput(const TextInput& textInput);
        TextInput textInput() const;

        void addSkipHour(int hour);
        void setSkipHours(const QList<int>& skipHours);
        void removeSkipHour(int hour);
        QList<int> skipHours() const;

        void addSkipDay(int day);
        void setSkipDays(const QList<int>& skipDays);
        void removeSkipDay(int day);
        QList<int> skipDays() const;

        void addItem(const Item& item);
        void setItems(const QList<Item>& items);
        void removeItem(const Item& item);
        QList<Item> items() const;

        QString debugInfo() const;

        private:
        static Document* m_null;
        class DocumentPrivate;
        DocumentPrivate* d;
    };
} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_DOCUMENT_H
