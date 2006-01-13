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

#include "category.h"
#include "document.h"
#include "entry.h"
#include "link.h"
#include "person.h"

#include <QList>
#include <QString>

namespace LibSyndication {
namespace Atom {

FeedDocument::FeedDocument() : ElementWrapper()
{
}

FeedDocument::FeedDocument(const QDomElement& element)  : ElementWrapper(element)
{
}

bool FeedDocument::accept(DocumentVisitor* visitor)
{
    return true; // TODO
}

QList<Person> FeedDocument::authors() const
{
    return QList<Person>(); // TODO
}

QList<Person> FeedDocument::contributors() const
{
    return QList<Person>(); // TODO
}

QList<Category> FeedDocument::categories() const
{
    return QList<Category>(); // TODO
}

QString FeedDocument::icon() const
{
    return "TODO";
}

QString FeedDocument::logo() const
{
    return "TODO";
}

QString FeedDocument::id() const
{
    return "TODO";
}

QString FeedDocument::rights() const
{
    return "TODO";
}

QString FeedDocument::title() const
{
    return "TODO";
}

QString FeedDocument::subtitle() const
{
    return "TODO";
}

time_t FeedDocument::updated() const
{
    return 0; // TODO
}

QList<Link> FeedDocument::links() const
{
    return QList<Link>(); // TODO
}

QList<Entry> FeedDocument::entries() const
{
    return QList<Entry>(); // TODO
}

QString FeedDocument::debugInfo() const
{
    return "TODO";
}

EntryDocument::EntryDocument() : ElementWrapper()
{
}

EntryDocument::EntryDocument(const QDomElement& element)  : ElementWrapper(element)
{
}

bool EntryDocument::accept(DocumentVisitor* visitor)
{
    return true; // TODO
}

Entry EntryDocument::entry() const
{
    return Entry(); // TODO
}

QString EntryDocument::debugInfo() const
{
    return "TODO";
}

} // namespace Atom
} // namespace LibSyndication
