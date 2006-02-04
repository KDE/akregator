/*
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */

#ifndef LIBSYNDICATION_DATARETRIEVER_H
#define LIBSYNDICATION_DATARETRIEVER_H

#include <kdepimmacros.h>

#include <QObject>
#include <QString>

namespace KIO
{
    class Job;
}

class KProcess;
class KUrl;

class QByteArray;

namespace LibSyndication
{

/**
 * Abstract baseclass for all data retriever classes. Subclass this to add
 * a new retrieval algorithm which can then be plugged into the RSS loader.
 * @see Loader, FileRetriever, OutputRetriever
 */
class KDE_EXPORT DataRetriever : public QObject
{
    Q_OBJECT
    public:
        /**
         * Default constructor.
         */
        DataRetriever();
    
        /**
         * Destructor.
         */
        virtual ~DataRetriever();
    
        /**
         * Retrieve data from the given URL. This method is supposed to get
         * reimplemented by subclasses. It will be called by the Loader
         * class in case it needs to retrieve the data.
         * @see Loader::loadFrom()
         */
        virtual void retrieveData(const KUrl& url) = 0;
    
        /**
         * @return An error code which might give a more precise information
         * about what went wrong in case the 'success' flag returned with
         * the dataRetrieved() signal was 'false'. Note that the meaning of
         * the returned integer depends on the actual data retriever.
         */
        virtual int errorCode() const = 0;
    
        /**
         * aborts the retrieval process.
         */
        virtual void abort() = 0;
        
    signals:
        /**
         * Emit this signal to tell the Loader class that the retrieval
         * process was finished.
         * @param data Should contain the retrieved data and will get
         * parsed by the Loader class.
         * @param success Indicates whether there were any problems during
         * the retrieval process. Pass 'true' to indicate that everything
         * went seamlessy, 'false' to tell the Loader that something went
         * wrong and that the data parameter might contain no or invalid
         * data.
         */

        void dataRetrieved(const QByteArray& data, bool success);

    private:
        DataRetriever(const DataRetriever& other);
        DataRetriever& operator=(const DataRetriever& other);
};


/**
 * Implements a data retriever which executes a program and stores returned
 * by the program on stdout. To be used with Loader::loadFrom().
 * @see DataRetriever, Loader::loadFrom()
 */
class OutputRetriever : public DataRetriever
{
    Q_OBJECT
            
    public:
                
        /**
         * Default constructor.
         */
        OutputRetriever();

        /**
         * Destructor.
         */
        virtual ~OutputRetriever();

        /**
         * Executes the program referenced by the given URL and retrieves
         * the data which the program prints to stdout.
         * @param url An URL which is supposed to reference an executable
         * file.
         * @see Loader::loadFrom()
         */
        virtual void retrieveData(const KUrl& url);

        /**
         * @return The error code for the last process of retrieving data.
         * 0 is returned in case there was no error, otherwise an error
         * code which depends on the particular program which was run is
         * returned.
         */
        virtual int errorCode() const;

        virtual void abort() {}

    private slots:
        void slotOutput(KProcess* process, char* data, int length);
        void slotExited(KProcess* process);

    private:
        OutputRetriever(const OutputRetriever& other);
        OutputRetriever& operator=(const OutputRetriever& other);

        struct OutputRetrieverPrivate;
        OutputRetrieverPrivate* d;
};

/**
 * Implements a file retriever, to be used with Loader::loadFrom().
 * @see DataRetriever, Loader::loadFrom()
 */
class KDE_EXPORT FileRetriever : public DataRetriever
{
    Q_OBJECT
            
    public:
        
        /**
         * Default constructor.
         */
        FileRetriever();

        /**
         * Destructor.
         */
        virtual ~FileRetriever();

        /**
         * Downloads the file referenced by the given URL and passes it's
         * contents on to the Loader.
         * @param url An URL referencing a file which is assumed to
         * reference valid XML.
         * @see Loader::loadFrom()
         */
        virtual void retrieveData(const KUrl& url);

        /**
         * @return The error code for the last process of retrieving data.
         * The returned numbers correspond directly to the error codes
         * <a href="http://developer.kde.org/documentation/library/cvs-api/classref/kio/KIO.html#Error">as
         * defined by KIO</a>.
         */
        virtual int errorCode() const;

        virtual void abort();

        static void setUseCache(bool enabled);
        static void setUserAgent(const QString& userAgent);

    signals:
        
        /**
         * Signals a permanent redirection. The redirection itself is
         * handled internally, so you don't need to call Loader::loadFrom()
         * with the new URL. This signal is useful in case you want to
         * notify the user, or adjust a database entry.
         * @see Loader::loadFrom()
         */
        void permanentRedirection(const KUrl& url);

    protected slots:
        
        void slotTimeout();

    private slots:
        
        void slotData(KIO::Job*job, const QByteArray& data);
        void slotResult(KIO::Job* job);
        void slotPermanentRedirection(KIO::Job* job, const KUrl& fromUrl,
                                      const KUrl& toUrl);

    private:
        
        static bool m_useCache;
        static QString m_userAgent;

        FileRetriever(const FileRetriever& other);
        FileRetriever& operator=(const FileRetriever& other);

        struct FileRetrieverPrivate;
        FileRetrieverPrivate* d;
};

} // namespace LibSyndication

#endif // LIBSYNDICATION_DATARETRIEVER_H
