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

#ifndef AKREGATOR_DRAGOBJECTS_H
#define AKREGATOR_DRAGOBJECTS_H

#include "article.h"

#include <kurl.h>
#include <k3urldrag.h>

#include <QList>
#include <QString>

namespace Akregator {

class Article;

struct ArticleDragItem
{
    QString feedURL;
    QString guid;
};

class ArticleDrag : public K3URLDrag
{

public:

    ArticleDrag(const QList<Article>& articles, QWidget* dragSource=0, const char* name=0);

    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, QList<ArticleDragItem>& articles);

protected:

    virtual QByteArray encodedData(const char* mime) const;
    virtual const char* format(int i) const;

private:

    static QList<ArticleDragItem> articlesToDragItems(const QList<Article>& articles);
    static KURL::List articleURLs(const QList<Article>& articles);
    QList<ArticleDragItem> m_items;
};

} // namespace Akregator

#endif // AKREGATOR_DRAGOBJECTS_H
