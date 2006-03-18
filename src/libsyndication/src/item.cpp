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
#include "enclosure.h"
#include "item.h"
#include "person.h"
#include "tools.h"

#include <QList>
#include <QString>

namespace LibSyndication {

QString Item::debugInfo() const
{
    QString info;

    info += "# Item begin ######################\n";
    
    QString did = id();
    if (!did.isNull())
        info += "id: #" + did + "#\n";

    QString dtitle = title();
    if (!dtitle.isNull())
        info += "title: #" + dtitle + "#\n";
    
    QString dlink = link();
    if (!dlink.isNull())
        info += "link: #" + dlink + "#\n";
    
    QString ddescription = description();
    if (!ddescription.isNull())
        info += "description: #" + ddescription + "#\n";
    
    QString dcontent = content();
    if (!dcontent.isNull())
        info += "content: #" + dcontent + "#\n";
    
    QString pubdate = dateTimeToString(datePublished());
    if (!pubdate.isNull())
        info += "datePublished: #" + pubdate + "#\n";
    
    QString update = dateTimeToString(dateUpdated());
    if (!update.isNull())
        info += "dateUpdated: #" + update + "#\n";

    QString dlanguage = language();
    if (!dlanguage.isNull())
        info += "language: #" + dlanguage + "#\n";
    
    QList<PersonPtr> dauthors = authors();
    QList<PersonPtr>::ConstIterator itp = dauthors.begin();
    QList<PersonPtr>::ConstIterator endp = dauthors.end();
    
    for ( ; itp != endp; ++itp)
        info += (*itp)->debugInfo();

    QList<CategoryPtr> dcategories = categories();
    QList<CategoryPtr>::ConstIterator itc = dcategories.begin();
    QList<CategoryPtr>::ConstIterator endc = dcategories.end();
    
    for ( ; itc != endc; ++itc)
        info += (*itc)->debugInfo();

    QList<EnclosurePtr> denclosures = enclosures();
    QList<EnclosurePtr>::ConstIterator ite = denclosures.begin();
    QList<EnclosurePtr>::ConstIterator ende = denclosures.end();
    
    for ( ; ite != ende; ++ite)
        info += (*ite)->debugInfo();

    int dcommentsCount = commentsCount();
    if (dcommentsCount != -1)
    {
        info+= "commentsCount: #" + QString::number(dcommentsCount) + "#\n";
    }
    
    QString dcommentsLink = commentsLink();
    if (!dcommentsLink.isNull())
        info+= "commentsLink: #" + dcommentsLink + "#\n";
    
    QString dcommentsFeed = commentsFeed();
    if (!dcommentsFeed.isNull())
        info+= "commentsFeed: #" + dcommentsFeed + "#\n";
    
    QString dcommentPostUri = commentPostUri();
    if (!dcommentPostUri.isNull())
        info+= "commentPostUri: #" + dcommentPostUri + "#\n";
    
    info += "# Item end ########################\n";

    return info;
}

} // namespace LibSyndication
