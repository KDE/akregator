/*
    This file is part of Akregator.

    Copyright (C) 2005-2007 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#ifndef AKREGATOR_ARTICLEINTERCEPTOR_H
#define AKREGATOR_ARTICLEINTERCEPTOR_H

#include "akregator_export.h"

template <class T> class QList;

namespace Akregator {

class Article;

/** Interface for intercepting new articles which were just  
 * fetched before adding them to the archive. E.g. an article
 * filter could implement this interface to get fetched
 * articles and label them.
 */

class AKREGATOR_EXPORT ArticleInterceptor
{
    public:
        virtual ~ArticleInterceptor() {}

        /** 
         * processes an article. Note that the interceptor
         * may modify the article 
         */
        virtual void processArticle(Article& article) = 0;

};


/** 
 * Singleton managing the interceptors. Added interceptors 
 * will receive fetched articles.
 */
class AKREGATOR_EXPORT ArticleInterceptorManager
{
    public:

        /** returns the singleton instance managing the interceptors */
        static ArticleInterceptorManager* self();

        ArticleInterceptorManager();
        ~ArticleInterceptorManager();

        /** adds an interceptor. When added, the interceptor receives 
         * fetches articles. 
         */
        void addInterceptor(ArticleInterceptor* interceptor);

        /** 
         * removes the interceptor. After being removed, the interceptor 
         * will not longer receive fetched articles 
         */
        void removeInterceptor(ArticleInterceptor* interceptor);

        /** returns the list of registered interceptors */
        QList<ArticleInterceptor*> interceptors() const;

    private:
       static ArticleInterceptorManager* m_self;

       class ArticleInterceptorManagerPrivate;
       ArticleInterceptorManagerPrivate* d;
};

} //namespace Akregator

#endif // AKREGATOR_ARTICLEINTERCEPTOR_H
