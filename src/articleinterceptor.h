#ifndef AKREGATOR_ARTICLEINTERCEPTOR_H
#define AKREGATOR_ARTICLEINTERCEPTOR_H

#include "akregator_export.h"

template <class T> class QValueList;

namespace Akregator {

class Article;

/** Interface for intercepting new articles which were just fetched before adding 
 *  them to the archive. E.g. an article filter could implement this interface to 
 *  get fetched articles and label them */

class AKREGATOR_EXPORT ArticleInterceptor
{
    public:
        /** processes an article. Note that the interceptor may modify the article */
        virtual void processArticle(Article& article) = 0;

};

/** Singleton managing the interceptors */
class AKREGATOR_EXPORT ArticleInterceptorManager
{
    public:

        static ArticleInterceptorManager* self();

        ArticleInterceptorManager();
        virtual ~ArticleInterceptorManager();

        void addInterceptor(ArticleInterceptor* interceptor);
        void removeInterceptor(ArticleInterceptor* interceptor);
        QValueList<ArticleInterceptor*> interceptors() const;

    private:
       static ArticleInterceptorManager* m_self;
       class ArticleInterceptorManagerPrivate;
       ArticleInterceptorManagerPrivate* d;
};

} //namespace Akregator

#endif // AKREGATOR_ARTICLEINTERCEPTOR_H
