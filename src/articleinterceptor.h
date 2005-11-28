#ifndef AKREGATOR_ARTICLEINTERCEPTOR_H
#define AKREGATOR_ARTICLEINTERCEPTOR_H

template <class T> class QList;

namespace Akregator {

class Article;

/** Interface for intercepting new articles which were just  
 * fetched before adding them to the archive. E.g. an article
 * filter could implement this interface to get fetched
 * articles and label them.
 */

class ArticleInterceptor
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
class ArticleInterceptorManager
{
    public:

        /** returns the singleton instance managing the interceptors */
        static ArticleInterceptorManager* self();

        ArticleInterceptorManager();
        virtual ~ArticleInterceptorManager();

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
