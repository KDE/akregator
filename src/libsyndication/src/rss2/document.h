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
template <class T> class QSet;

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



        /**
         * The title of the channel.
         *
         * @return title TODO: more on escaping/HTML
         */
        QString title() const;

        void setTitle(const QString& title);

        /** 
         * The URL to the HTML website corresponding to the channel.
         *
         * @return TODO 
         */
        QString link() const;

        void setLink(const QString& link);

        /**
         * Phrase or sentence describing the channel.
         *
         * @return TODO
         */
        QString description() const;

        void setDescription(const QString& description);

        /**
         *
         * @return TODO
         */
        QString language() const;
        void setLanguage(const QString& language);

        /**
         * Copyright notice for content in the channel.
         *
         * @return TODO
         */
        QString copyright() const;
        void setCopyright(const QString& copyright);

        /**
         * Email address for person responsible for editorial content.
         *
         * @return TODO
         */
        QString managingEditor() const;
        void setManagingEditor(const QString& managingEditor);

        /**
         * Email address for person responsible for technical issues relating
         * to channel.
         *
         * @return TODO
         */
        QString webMaster() const;
        void setWebMaster(const QString& webMaster);

        /** 
         * The publication date for the content in the channel. For example,
         * the New York Times publishes on a daily basis, the publication date
         * flips once every 24 hours. That's when the pubDate of the channel
         * changes.
         *
         * @return TODO
         */
        QDateTime pubDate() const;
        void setPubDate(const QDateTime& pubDate);

        /**
         * The last time the content of the channel changed.
         *
         * @return TODO
         */
        QDateTime lastBuildDate() const;
        void setLastBuildDate(const QDateTime& lastBuildDate);

        /**
         * Specifies one or more categories that the channel belongs to.
         *
         * @return TODO
         */
        QList<Category> categories() const;
        void addCategory(const Category& category);
        void setCategories(const QList<Category>& categories);
        void removeCategory(const Category& category);

        /**
         * A string indicating the program used to generate the channel.
         *
         * @return TODO
         */
        QString generator() const;
        void setGenerator(const QString& generator);

        /**
         * A URL that points to the documentation for the format used in the
         * RSS file. It's probably a pointer to this page. It's for people who
         * might stumble across an RSS file on a Web server 25 years from now
         * and wonder what it is.
         *
         * @return TODO
         */
        QString docs() const;
        void setDocs(const QString& docs);

        /**
         * Allows processes to register with a cloud to be notified of updates
         * to the channel, implementing a lightweight publish-subscribe
         * protocol for RSS feeds.
         *
         * @return TODO
         */
        Cloud cloud() const;
        void setCloud(const Cloud& cloud);

        /**
         * ttl stands for time to live. It's a number of minutes that indicates
         * how long a channel can be cached before refreshing from the source.
         *
         * @return TODO
         */
        int ttl() const;
        void setTtl(int ttl);

        /**
         * Specifies a GIF, JPEG or PNG image that can be displayed with the
         * channel.
         *
         * @return TODO
         */
        Image image() const;
        void setImage(const Image& image);

        /**
         * Specifies a text input box that can be displayed with the channel.
         *
         * @return TODO
         */
        TextInput textInput() const;
        void setTextInput(const TextInput& textInput);

        /**
         * Contains a set of hours (from 0 to 23), time in GMT, when the
         * channel is not updated.
         *
         * @return TODO
         */
        QSet<int> skipHours() const;
        void addSkipHour(int hour);
        void setSkipHours(const QSet<int>& skipHours);
        void removeSkipHour(int hour);

        /** days of week, used for skip list */
        enum DayOfWeek
        {
            
            Monday = 0, /**< self-explanatory */
            Tuesday = 1, /**< self-explanatory */
            Wednesday = 2, /**< self-explanatory */
            Thursday = 3, /**< self-explanatory */
            Friday = 4, /**< self-explanatory */
            Saturday = 5, /**< self-explanatory */
            Sunday = 6 /**< self-explanatory */
        };

        /**
         * A set of week days where aggregators shouldn't read the channel.
         *
         * @return TODO
         */
        QSet<DayOfWeek> skipDays() const;
        void addSkipDay(DayOfWeek day);
        void setSkipDays(const QSet<DayOfWeek>& skipDays);
        void removeSkipDay(DayOfWeek day);

        /**
         *
         *
         * @return TODO
         */
        QList<Item> items() const;
        void addItem(const Item& item);
        void setItems(const QList<Item>& items);
        void removeItem(const Item& item);

        QString debugInfo() const;

        private:
        static Document* m_null;
        class DocumentPrivate;
        DocumentPrivate* d;
    };
} // namespace RSS2
} // namespace LibSyndication

#endif // LIBSYNDICATION_RSS2_DOCUMENT_H
