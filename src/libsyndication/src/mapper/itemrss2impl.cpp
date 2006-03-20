
/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "categoryrss2impl.h"
#include "enclosurerss2impl.h"
#include "itemrss2impl.h"
#include "personimpl.h"
#include "../rss2/category.h"
#include "../rss2/enclosure.h"
#include "../constants.h"
#include "../tools.h"

#include <QList>
#include <QString>

namespace LibSyndication {

ItemRSS2Impl::ItemRSS2Impl(const LibSyndication::RSS2::Item& item) : m_item(item)
{
}

QString ItemRSS2Impl::title() const
{
    return m_item.title();
}

QString ItemRSS2Impl::link() const
{
    
    QString guid = m_item.guid();
    if (!guid.isEmpty() && m_item.guidIsPermaLink())
        return guid;
    else
        return m_item.link();
}

QString ItemRSS2Impl::description() const
{
    return m_item.description();
}

QString ItemRSS2Impl::content() const
{
    return m_item.content();
}

QList<PersonPtr> ItemRSS2Impl::authors() const
{
    QList<PersonPtr> list;
    
    PersonPtr ptr = PersonImpl::fromString(m_item.author());
    
    if (!ptr->isNull())
    {
        list.append(ptr);
    }
    
    return list;
}

QString ItemRSS2Impl::language() const
{
    return QString::null;
}

QString ItemRSS2Impl::id() const
{
    QString guid = m_item.guid();
    if (!guid.isEmpty())
        return guid;
    
    return QString("hash:%1").arg(calcMD5Sum(title() 
            + description() + content()));
}

time_t ItemRSS2Impl::datePublished() const
{
    return m_item.pubDate();
}

time_t ItemRSS2Impl::dateUpdated() const
{
    return datePublished();
}

QList<LibSyndication::EnclosurePtr> ItemRSS2Impl::enclosures() const
{
    QList<LibSyndication::EnclosurePtr> list;
    
    QList<LibSyndication::RSS2::Enclosure> encs = m_item.enclosures();
    
    for (QList<LibSyndication::RSS2::Enclosure>::ConstIterator it = encs.begin();
         it != encs.end(); ++it)
    {
        EnclosureRSS2ImplPtr impl(new EnclosureRSS2Impl(m_item, *it));
        list.append(LibSyndication::EnclosurePtr::staticCast(impl));
    }
    
    return list;
}

QList<LibSyndication::CategoryPtr> ItemRSS2Impl::categories() const
{
    QList<LibSyndication::CategoryPtr> list;
    
    QList<LibSyndication::RSS2::Category> cats = m_item.categories();
    QList<LibSyndication::RSS2::Category>::ConstIterator it = cats.begin();
    QList<LibSyndication::RSS2::Category>::ConstIterator end = cats.end();
    
    for ( ; it != end; ++it)
    {
        CategoryRSS2ImplPtr impl(new CategoryRSS2Impl(*it));
        list.append(LibSyndication::CategoryPtr::staticCast(impl));
    }
    
    return list;
}

int ItemRSS2Impl::commentsCount() const
{
    QString cstr = m_item.extractElementTextNS(slashNamespace(), QString::fromUtf8("comments"));
    bool ok = false;
    int comments = cstr.toInt(&ok);
    return ok ? comments : -1;
}

QString ItemRSS2Impl::commentsLink() const
{
    return m_item.comments();
}

QString ItemRSS2Impl::commentsFeed() const
{
    return m_item.extractElementTextNS(commentApiNamespace(), QString::fromUtf8("commentRss"));
}

QString ItemRSS2Impl::commentPostUri() const
{
    return m_item.extractElementTextNS(commentApiNamespace(), QString::fromUtf8("comment"));
}

LibSyndication::SpecificItemPtr ItemRSS2Impl::specificItem() const
{
    return LibSyndication::SpecificItemPtr(new LibSyndication::RSS2::SpecificItem(m_item));
}

} // namespace LibSyndication
