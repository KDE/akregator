/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

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
};

#endif // AKREGATOR_ARTICLESEQUENCE_H
