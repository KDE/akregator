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
#include "constants.h"
#include "elementwrapper.h"

#include <kurl.h>

#include <QDomElement>
#include <QString>
#include <QTextStream>

namespace LibSyndication {

class ElementWrapper::ElementWrapperPrivate : public KShared
{
    public:
        
    QDomElement element;
    mutable QString xmlBase;
    mutable QString xmlLang;
};

ElementWrapper::ElementWrapper() : d(new ElementWrapperPrivate)
{
}

ElementWrapper::ElementWrapper(const ElementWrapper& other)
{
    *this = other;
}

ElementWrapper::ElementWrapper(const QDomElement& element) : d(new ElementWrapperPrivate)
{
    d->element = element;
}

ElementWrapper::~ElementWrapper()
{
}

ElementWrapper& ElementWrapper::operator=(const ElementWrapper& other)
{
    d = other.d;
    return *this;
}

bool ElementWrapper::operator==(const ElementWrapper& other) const
{
    return d->element == other.d->element;
}

bool ElementWrapper::isNull() const
{
    return d->element.isNull();
}

const QDomElement& ElementWrapper::element() const
{
    return d->element;
}

QString ElementWrapper::xmlBase() const
{
    if (d->xmlBase.isNull()) // xmlBase not computed yet
    {
        QDomElement current = d->element;
        
        while (!current.isNull())
        {
            if (current.hasAttributeNS(Constants::xmlNamespace(), QString::fromLatin1("base")))
            {
                d->xmlBase = current.attributeNS(Constants::xmlNamespace(), QString::fromLatin1("base"));
                return d->xmlBase;
            }
            
            QDomNode parent = current.parentNode();

            if (!parent.isNull() && parent.isElement())
                current = parent.toElement();
            else
                current = QDomElement();
        }
        
        // no xml:base found
        d->xmlBase = "";
        return "";
    }
    return d->xmlBase;
}

QString ElementWrapper::completeURI(const QString& uri) const
{
    KURL u(xmlBase(), uri);
    
    if (u.isValid())
        return u.url();
    
    return uri;
}

QString ElementWrapper::xmlLang() const
{
    if (d->xmlLang.isNull()) // xmlLang not computed yet
    {
        QDomElement current = d->element;
        
        while (!current.isNull())
        {
            if (current.hasAttributeNS(Constants::xmlNamespace(), QString::fromLatin1("base")))
            {
                d->xmlLang = current.attributeNS(Constants::xmlNamespace(), QString::fromLatin1("base"));
                return d->xmlLang;
            }
            
            QDomNode parent = current.parentNode();

            if (!parent.isNull() && parent.isElement())
                current = parent.toElement();
            else
                current = QDomElement();
        }
        
        // no xml:lang found
        d->xmlLang = "";
        return "";
    }
    return d->xmlLang;
}
        
QString ElementWrapper::extractElementText(const QString& tagName) const
{
    if (isNull())
        return QString::null;

    QDomNode node = d->element.namedItem(tagName);

    if (node.isNull() || !node.isElement())
        return QString::null;
    else
        return node.toElement().text();
}

QString ElementWrapper::childNodesAsXML(const QDomElement& parent)
{
    ElementWrapper wrapper(parent);
    
    if (parent.isNull())
        return QString::null;

    QDomNodeList list = parent.childNodes();
    
    QString str;
    QTextStream ts( &str, QIODevice::WriteOnly );
    
    // if there is a xml:base in our scope, first set it for
    // each child element so the xml:base shows up in the
    // serialization
    QString base = wrapper.xmlBase();


    for (int i = 0; i < list.count(); ++i)
    {
        QDomNode it = list.item(i);
        if (!base.isEmpty() && it.isElement() 
             && !it.toElement().hasAttributeNS(Constants::xmlNamespace(), QString::fromLatin1("base")))
        {
            it.toElement().setAttributeNS(Constants::xmlNamespace(), QString::fromLatin1("base"), base);
        }
            
        ts << it;
    }
    return str.trimmed();
}

QString ElementWrapper::childNodesAsXML() const
{
    return childNodesAsXML(d->element);
}

QString ElementWrapper::extractElementTextNS(const QString& namespaceURI, const QString& localName) const
{
    QDomElement el = firstElementByTagNameNS(namespaceURI, localName);
    
    if (el.isNull())
        return QString::null;

    return el.text().trimmed();
}

QList<QDomElement> ElementWrapper::elementsByTagName(const QString& tagName) const
{
    QList<QDomElement> elements;
    for (QDomNode n = d->element.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if (e.tagName() == tagName)
                elements.append(e);
        }
    }
    return elements;
}

QDomElement ElementWrapper::firstElementByTagNameNS(const QString& nsURI, const QString& localName) const
{
    if (isNull())
        return QDomElement();
    
    for (QDomNode n = d->element.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if (e.localName() == localName && e.namespaceURI() == nsURI)
                return e;
        }
    }
    
    return QDomElement();
}


QList<QDomElement> ElementWrapper::elementsByTagNameNS(const QString& nsURI, const QString& localName) const
{
    if (isNull())
        return QList<QDomElement>();
    
    QList<QDomElement> elements;
    for (QDomNode n = d->element.firstChild(); !n.isNull(); n = n.nextSibling())
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            if (e.localName() == localName && e.namespaceURI() == nsURI)
                elements.append(e);
        }
    }
    return elements;
}

} // namespace LibSyndication
