//Added by qt3to4:
#include <Q3ValueList>
#ifndef AKREGATOR_ARTICLEINTERCEPTOR_H
#define AKREGATOR_ARTICLEINTERCEPTOR_H

template <class T> class Q3ValueList;

namespace Akregator {

class Article;

/** Interface for intercepting new articles which were just fetched before adding them to the archive. E.g. an article filter could implement this interface to get fetched articles and label them */

class ArticleInterceptor
{
    public:
		virtual ~ArticleInterceptor(){}
        /** processes an article. Note that the interceptor may modify the article */
        virtual void processArticle(Article& article) = 0;

};

/** Singleton managing the interceptors */
class ArticleInterceptorManager
{
    public:

        static ArticleInterceptorManager* self();

        ArticleInterceptorManager();
        virtual ~ArticleInterceptorManager();

        void addInterceptor(ArticleInterceptor* interceptor);
        void removeInterceptor(ArticleInterceptor* interceptor);
        Q3ValueList<ArticleInterceptor*> interceptors() const;

    private:
       static ArticleInterceptorManager* m_self;
       class ArticleInterceptorManagerPrivate;
       ArticleInterceptorManagerPrivate* d;
};

} //namespace Akregator

#endif // AKREGATOR_ARTICLEINTERCEPTOR_H
