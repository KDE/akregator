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

#include <QList>

namespace Akregator {

class Article;

ArticleDrag::ArticleDrag(const QList<Article>& articles, QWidget* dragSource, const char* name)
: K3URLDrag(articleURLs(articles), dragSource), m_items(articlesToDragItems(articles))
{
    setObjectName(name);
}

bool ArticleDrag::canDecode(const QMimeSource* e)
{
    return e->provides("akregator/articles");
}

bool ArticleDrag::decode(const QMimeSource* e, QList<ArticleDragItem>& articles)
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
    QByteArray mimetype(mime);
    if (mimetype == "akregator/articles")
    {
        QByteArray ba;
        QDataStream stream( &ba,QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_3_1);

        QList<ArticleDragItem>::ConstIterator end = m_items.end();
        for (QList<ArticleDragItem>::ConstIterator it = m_items.begin(); it != end; ++it)
        {
            stream << (*it).feedURL;
            stream << (*it).guid;
        } 
        return ba;
    }
    else
    {
        return K3URLDrag::encodedData(mime);
    }
}

QList<ArticleDragItem> ArticleDrag::articlesToDragItems(const QList<Article>& articles)
{
    QList<ArticleDragItem> items;
    
    QList<Article>::ConstIterator end(articles.end());

    for (QList<Article>::ConstIterator it = articles.begin(); it != end; ++it)
    {
        ArticleDragItem i;
        i.feedURL = (*it).feed() ? (*it).feed()->xmlUrl() : "";
        i.guid = (*it).guid();
        items.append(i);
    }

    return items;
}

KURL::List ArticleDrag::articleURLs(const QList<Article>& articles)
{
    KURL::List urls;
    QList<Article>::ConstIterator end(articles.end());
    for (QList<Article>::ConstIterator it = articles.begin(); it != end; ++it)
        urls.append((*it).link());
    return urls;
}

} // namespace Akregator
