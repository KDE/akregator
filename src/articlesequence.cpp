/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
 
#include "articlesequence.h"

using namespace Akregator;

// = ArticleList ===================================================== //

struct ArticleList::Private
{
   int dummy;
   bool doSort :1;
};

ArticleList::ArticleList()
   : Article::List()
   , d(new Private)
{
}

ArticleList::ArticleList(const ArticleList &other)
   : Article::List(other)
   , d(new Private)
{
}

ArticleList::~ArticleList()
{
   delete d;
}

/*
    The reason to include insert/append/prepend code here is to:
    a) check if there's another record with the exactly same pubDate() present,
    b) if so, adjust this inserted item's clock off by one second to keep sorting sane,
    c) re-sort added items (if enabled).
    d) use Article::fetchDate for sorting! ( defined by Article::operator <() )
 */
ArticleList::iterator ArticleList::insert( iterator it, const Article &x )
{
    return Article::List::insert( it, x );
}

void ArticleList::insert( iterator it, size_type n, const Article &x )
{
    Article::List::insert( it, n, x );
}

ArticleList::iterator ArticleList::append( const Article &x )
{
    return Article::List::append( x );
}

ArticleList::iterator ArticleList::prepend( const Article &x )
{
    return Article::List::prepend( x );
}


void ArticleList::enableSorting(bool b)
{
    d->doSort = b;
}

void ArticleList::sort()
{
    if (d->doSort)
    {
        qHeapSort( *this );
    }
}


