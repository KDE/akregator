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
    d->interceptors.remove(interceptor);
}

QList<ArticleInterceptor*> ArticleInterceptorManager::interceptors() const
{
    return d->interceptors;
}

}

