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

#include "article.h"
#include "articleinterceptor.h"

#include <QList>
#include <kstaticdeleter.h>

namespace Akregator
{

class ArticleInterceptorManager::ArticleInterceptorManagerPrivate 
{
    public:
        QList<ArticleInterceptor*> interceptors;
};


ArticleInterceptorManager* ArticleInterceptorManager::m_self = 0;
KStaticDeleter<ArticleInterceptorManager> interceptormanagersd;

ArticleInterceptorManager* ArticleInterceptorManager::self()
{
    if (!m_self)
        interceptormanagersd.setObject(m_self, new ArticleInterceptorManager);
    return m_self;
}

ArticleInterceptorManager::~ArticleInterceptorManager() 
{
    delete d; 
    d = 0;
}

ArticleInterceptorManager::ArticleInterceptorManager() : d(new ArticleInterceptorManagerPrivate)
{}

void ArticleInterceptorManager::addInterceptor(ArticleInterceptor* interceptor)
{
    d->interceptors.append(interceptor);
}

void ArticleInterceptorManager::removeInterceptor(ArticleInterceptor* interceptor)
{
    d->interceptors.removeAll(interceptor);
}

QList<ArticleInterceptor*> ArticleInterceptorManager::interceptors() const
{
    return d->interceptors;
}

}

