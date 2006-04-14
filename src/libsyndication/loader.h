/*
 * loader.h
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */
#ifndef LIBSYNDICATION_LOADER_H
#define LIBSYNDICATION_LOADER_H

#include "global.h"

#include <kdepimmacros.h>

#include "sharedptr.h"

#include <QObject>

class KProcess;
class KUrl;

namespace KIO
{
    class Job;
}

namespace LibSyndication {

class DataRetriever;
class Feed;
typedef SharedPtr<Feed> FeedPtr;

/**
 * This class is the preferred way of loading feed sources. Usage is very
 * straightforward:
 *
 * \code
 * Loader *loader = Loader::create();
 * connect(loader, SIGNAL(loadingComplete(Loader*, FeedPtr, ErrorCode)),
 *         this, SLOT(slotLoadingComplete(Loader*, FeedPtr, ErrorCode)));
 * loader->loadFrom("http://www.blah.org/foobar.rdf");
 * \endcode
 *
 * This creates a Loader object, connects it's loadingComplete() signal to
 * your custom slot and then makes it load the file
 * 'http://www.blah.org/foobar.rdf'. You could've
 * done something like this as well:
 *
 * \code
 * // create the Loader, connect it's signal...
 * loader->loadFrom("/home/myself/some-script.py", new OutputRetriever);
 * \endcode
 *
 * That'd make the Loader use a custom algorithm for retrieving the RSS data;
 * 'OutputRetriever' will make it execute the script
 * '/home/myself/some-script.py' and assume whatever that script prints to
 * stdout is RSS/Azom markup. This is e.g. handy for conversion scripts, which
 * download a HTML file and convert it's contents into RSS markup.
 *
 * No matter what kind of retrieval algorithm you employ, your
 * 'slotLoadingComplete' method might look like this:
 *
 * \code
 * void MyClass::slotLoadingComplete(Loader* loader, FeedPtr feed, ErrorCode status)
 * {
 *     // Note that Loader::~Loader() is private, so you cannot delete Loader instances.
 *     // You don't need to do that anyway since Loader instances delete themselves.
 *
 *     if (status != LibSyndication::Success)
 *         return;
 *
 *     QString title = feed->title();
 *     // do whatever you want with the information.
 * }
 * \endcode
 */
class KDE_EXPORT Loader : public QObject
{
    Q_OBJECT
    
    
    public:
        
        /**
         * Constructs a Loader instance. This is pretty much what the
         * default constructor would do, except that it ensures that all
         * Loader instances have been allocated on the heap (this is
         * required so that Loader's can delete themselves safely after they
         * emitted the loadingComplete() signal.).
         * @return A pointer to a new Loader instance.
         */
        static Loader* create();
    
        /**
         * Convenience method. Does the same as the above method except that
         * it also does the job of connecting the loadingComplete() signal
         * to the given slot for you.
         * @param object A QObject which features the specified slot
         * @param slot Which slot to connect to.
         */
        static Loader* create(QObject* object, const char* slot);
    
        /**
         * Loads the feed source referenced by the given URL using the
         * specified retrieval algorithm. Make sure that you connected
         * to the loadingComplete() signal before calling this method so
         * that you're guaranteed to get notified when the loading finished.
         * \note A Loader object cannot load from multiple URLs simultaneously;
         * consequently, subsequent calls to loadFrom will be discarded
         * silently, only the first loadFrom request will be executed.
         * @param url A URL referencing the input file.
         * @param retriever A subclass of DataRetriever which implements a
         * specialized retrieval behaviour. Note that the ownership of the
         * retriever is transferred to the Loader, i.e. the Loader will
         * delete it when it doesn't need it anymore.
         * @see DataRetriever, Loader::loadingComplete()
         */
        void loadFrom(const KUrl& url, DataRetriever* retriever);
    
        /**
         * Convenience method. Does the same as the above method, where
         * FileRetriever is used as retriever implementation.
         * 
         * @param url A URL referencing the input file.
         */
        void loadFrom(const KUrl& url);
        
        /**
         * Retrieves the error code of the last loading process (if any).
         */
        ErrorCode errorCode() const;
    
        /**
         * the error code returned from the retriever.
         * Use this if you use your custom retriever implementation and
         * need the specific error, not covered by errorCode().
         */
        int retrieverError() const;
        
        const KUrl& discoveredFeedURL() const;
    
        void abort();
    
    signals:
        

        /**
         * This signal gets emitted when the loading process triggered by
         * calling loadFrom() finished.
         * @param loader A pointer pointing to the loader object which
         * emitted this signal; this is handy in case you connect multiple
         * loaders to a single slot.
         * @param feed In case errortus is Success, this parameter holds the
         * parsed feed. If fetching/parsing failed, feed is NULL.
         * @param error An error code telling whether there were any
         * problems while retrieving or parsing the data.
         * @see Feed, ErrorCode
         */
        void loadingComplete(LibSyndication::Loader* loader,
                             LibSyndication::FeedPtr feed,
                             LibSyndication::ErrorCode error);

    private slots:
        
        void slotRetrieverDone(const QByteArray& data, bool success);

    private:
        
        Loader();
        Loader(const Loader& other);
        Loader& operator=(const Loader& other);
        ~Loader();
        void discoverFeeds(const QByteArray& data);

        struct LoaderPrivate;
        LoaderPrivate* d;
};


} // namespace LibSyndication

#endif // LIBSYNDICATION_LOADER_H
