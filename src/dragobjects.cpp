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

#include "dragobjects.h"
#include "feed.h"

#include <q3cstring.h>
//Added by qt3to4:
#include <Q3ValueList>

namespace Akregator {

class Article;

ArticleDrag::ArticleDrag(const Q3ValueList<Article>& articles, QWidget* dragSource, const char* name)
: KURLDrag(articleURLs(articles), dragSource, name), m_items(articlesToDragItems(articles))
{}

bool ArticleDrag::canDecode(const QMimeSource* e)
{
    return e->provides("akregator/articles");
}

bool ArticleDrag::decode(const QMimeSource* e, Q3ValueList<ArticleDragItem>& articles)
{
    articles.clear();
    QByteArray array = e->encodedData("akregator/articles");
    
    QDataStream stream( &array,QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_3_1);

    while (!stream.atEnd())
    {
        ArticleDragItem i;
        stream >> i.feedURL;
        stream >> i.guid;
        articles.append(i);
    }

    return true;
}

const char* ArticleDrag::format(int i) const
{
    if (i == 0)
        return "text/uri-list";
    else if (i == 1)
        return "akregator/articles";

    return 0;
}

QByteArray ArticleDrag::encodedData(const char* mime) const
{
    Q3CString mimetype(mime);
    if (mimetype == "akregator/articles")
    {
        QByteArray ba;
        QDataStream stream( &ba,QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_3_1);

        Q3ValueList<ArticleDragItem>::ConstIterator end = m_items.end();
        for (Q3ValueList<ArticleDragItem>::ConstIterator it = m_items.begin(); it != end; ++it)
        {
            stream << (*it).feedURL;
            stream << (*it).guid;
        } 
        return ba;
    }
    else
    {
        return KURLDrag::encodedData(mime);
    }
}

Q3ValueList<ArticleDragItem> ArticleDrag::articlesToDragItems(const Q3ValueList<Article>& articles)
{
    Q3ValueList<ArticleDragItem> items;
    
    Q3ValueList<Article>::ConstIterator end(articles.end());

    for (Q3ValueList<Article>::ConstIterator it = articles.begin(); it != end; ++it)
    {
        ArticleDragItem i;
        i.feedURL = (*it).feed() ? (*it).feed()->xmlUrl() : "";
        i.guid = (*it).guid();
        items.append(i);
    }

    return items;
}

KURL::List ArticleDrag::articleURLs(const Q3ValueList<Article>& articles)
{
    KURL::List urls;
    Q3ValueList<Article>::ConstIterator end(articles.end());
    for (Q3ValueList<Article>::ConstIterator it = articles.begin(); it != end; ++it)
        urls.append((*it).link());
    return urls;
}

} // namespace Akregator
