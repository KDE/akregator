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

#ifndef AKREGATOR_ARTICLESEQUENCE_H
#define AKREGATOR_ARTICLESEQUENCE_H

#include "myarticle.h"

namespace Akregator
{
    /** This is article list supporting sorting added articles by pubDate
     *  and properly emulating pubDate for articles with invalid pubDates.
     */
    class ArticleSequence : public MyArticle::List
    {
        public:

            ArticleSequence();
            ArticleSequence(const ArticleSequence &other);
            virtual ~ArticleSequence();

            iterator insert( iterator it, const MyArticle &x );
            void insert( iterator it, size_type n, const MyArticle &x );
            iterator append( const MyArticle &x );
            iterator prepend( const MyArticle &x );

            void enableSorting(bool b);
            void sort();
        
        private:
            struct Private;
            Private *d;
    };
}

#endif // AKREGATOR_ARTICLESEQUENCE_H
