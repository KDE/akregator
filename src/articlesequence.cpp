/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
 
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


