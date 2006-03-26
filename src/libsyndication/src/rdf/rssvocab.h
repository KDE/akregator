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
#ifndef LIBSYNDICATION_RDF_RSSVOCAB_H
#define LIBSYNDICATION_RDF_RSSVOCAB_H

#include <kdepimmacros.h>

class QString;
class QStringList;

namespace LibSyndication {
    
template <class T> class SharedPtr;

namespace RDF {

class Property;
typedef SharedPtr<Property> PropertyPtr;

class Resource;
typedef SharedPtr<Resource> ResourcePtr;

/**
 * Singleton holding RDF class and property constants of the RSS 1.0
 * vocabulary. See http://web.resource.org/rss/1.0/ for a specification.
 * 
 * @author Frank Osterfeld
 */
class KDE_EXPORT RSSVocab
{
    public:

        /**
         * virtual destructor
         */
        virtual ~RSSVocab();
        
        /**
         * returns the singleton instance
         */
        static RSSVocab* self();
        
        /**
         * namespace URI of the RSS 1.0 vocabulary,
         * @c "http://web.resource.org/rss/1.0/"
         */
        const QString& namespaceURI() const;

        /**
         * RSS 1.0 title property, see Document::title() for 
         * more details
         */
        PropertyPtr title() const;
        
        /**
         * RSS 1.0 description property, see Document::description() for 
         * more details
         */
        PropertyPtr description() const;
        
        /**
         * RSS 1.0 link property, see Document::link() for 
         * more details
         */
        PropertyPtr link() const;
        
        /**
         * RSS 1.0 name property, see Document::name() for 
         * more details
         */
        PropertyPtr name() const;
        
        /**
         * RSS 1.0 url property, see Document::url() for 
         * more details
         */
        PropertyPtr url() const;
        
        /**
         * RSS 1.0 channel class, the instance is represented by
         * LibSyndication::RDF::Document
         */
        ResourcePtr channel() const;
        
        /**
         * RSS 1.0 items property, see Document::items() for 
         * more details
         */
        PropertyPtr items() const;
        
        /**
         * RSS 1.0 image property, see Document::image() for 
         * more details
         */
        PropertyPtr image() const;
        
        /**
         * RSS 1.0 textinput property, see Document::textinput() for
         * more details
         */
        PropertyPtr textinput() const;
        
    protected:
        
        RSSVocab();
        
    private:
        
        static RSSVocab* m_self;
        
        class RSSVocabPrivate;
        RSSVocabPrivate* d;
};

/**
 * Singleton holding RDF class and property constants of the RSS 0.9
 * vocabulary. Only used to map RSS 0.9 to 1.0.
 * 
 * @author Frank Osterfeld
 */

class KDE_EXPORT RSS09Vocab
{
    public:

        /**
     * virtual destructor
         */
        virtual ~RSS09Vocab();
        
        /**
         * returns the singleton instance
         */
        static RSS09Vocab* self();
        
        /**
         * namespace URI of the RSS 0.9 vocabulary,
         * @c "http://web.resource.org/rss/0.9/"
         */
        const QString& namespaceURI() const;

        /**
         * RSS 0.9 title property, see Document::title() for 
         * more details
         */
        PropertyPtr title() const;
        
        /**
         * RSS 0.9 description property, see Document::description() for 
         * more details
         */
        PropertyPtr description() const;
        
        /**
         * RSS 0.9 link property, see Document::link() for 
         * more details
         */
        PropertyPtr link() const;
        
        /**
         * RSS 0.9 name property, see Document::name() for 
         * more details
         */
        PropertyPtr name() const;
        
        /**
         * RSS 0.9 url property, see Document::url() for 
         * more details
         */
        PropertyPtr url() const;
        
        /**
         * RSS 0.9 channel class, the instance is represented by
         * LibSyndication::RDF::Document
         */
        ResourcePtr channel() const;
        
        /**
         * RSS 0.9 item class, see Document::items() for 
         * more details
         */
        ResourcePtr item() const;
        
        /**
         * RSS 0.9 image property, see Document::image() for 
         * more details
         */
        PropertyPtr image() const;
        
        /**
         * RSS 0.9 textinput property, see Document::textinput() for
         * more details
         */
        PropertyPtr textinput() const;
        
        /**
         * returns a list containing all URIs representing properties in this vocabulary
         */
        QStringList properties() const;
        
        /**
         * returns a list containing all URIs representing classes in this vocabulary
         */
        QStringList classes() const;
        
    protected:
        
        RSS09Vocab();
        
    private:
        
        static RSS09Vocab* m_self;
        
        class RSS09VocabPrivate;
        RSS09VocabPrivate* d;
};

} // namespace RDF
} // namespace LibSyndication

#endif // LIBSYNDICATION_RDF_RSSVOCAB_H
