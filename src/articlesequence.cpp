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

// = ArticleSequence ===================================================== //

struct ArticleSequence::Private
{
   int dummy;
   bool doSort :1;
};

ArticleSequence::ArticleSequence()
   : MyArticle::List()
   , d(new Private)
{
}

ArticleSequence::ArticleSequence(const ArticleSequence &other)
   : MyArticle::List(other)
   , d(new Private)
{
}

ArticleSequence::~ArticleSequence()
{
   delete d;
}

/*
    The reason to include insert/append/prepend code here is to:
    a) check if there's another record with the exactly same pubDate() present,
    b) if so, adjust this inserted item's clock off by one second to keep sorting sane,
    c) re-sort added items (if enabled).
    d) use MyArticle::fetchDate for sorting! ( defined by MyArticle::operator <() )
 */
ArticleSequence::iterator ArticleSequence::insert( iterator it, const MyArticle &x )
{
    return MyArticle::List::insert( it, x );
}

void ArticleSequence::insert( iterator it, size_type n, const MyArticle &x )
{
    MyArticle::List::insert( it, n, x );
}

ArticleSequence::iterator ArticleSequence::append( const MyArticle &x )
{
    return MyArticle::List::append( x );
}

ArticleSequence::iterator ArticleSequence::prepend( const MyArticle &x )
{
    return MyArticle::List::prepend( x );
}


void ArticleSequence::enableSorting(bool b)
{
    d->doSort = b;
}

void ArticleSequence::sort()
{
    if (d->doSort)
    {
        qHeapSort( *this );
    }
}


