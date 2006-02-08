/*
 * dataretriever.cpp
 *
 * Copyright (c) 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. For licensing and distribution details, check the
 * accompanying file 'COPYING'.
 */

#include "dataretriever.h"
#include "global.h"

#include <kio/job.h>

#include <kprocess.h>
#include <kurl.h>

#include <QBuffer>
#include <QTimer>

namespace LibSyndication {

DataRetriever::DataRetriever()
{
}

DataRetriever::~DataRetriever()
{
}

struct FileRetriever::FileRetrieverPrivate
{
    FileRetrieverPrivate()
    : buffer(NULL),
    lastError(0), job(NULL)
    {
    }

    ~FileRetrieverPrivate()
    {
        delete buffer;
    }

    QBuffer *buffer;
    int lastError;
    KIO::Job *job;
};

FileRetriever::FileRetriever()
    : d(new FileRetrieverPrivate)
{
}

FileRetriever::~FileRetriever()
{
    delete d;
}

bool FileRetriever::m_useCache = true;
QString FileRetriever::m_userAgent = QString("LibSyndication %1").arg(LIBSYNDICATION_VERSION);

void FileRetriever::setUserAgent(const QString& userAgent)
{
    m_userAgent = userAgent;
}

void FileRetriever::setUseCache(bool enabled)
{
    m_useCache = enabled;
}

void FileRetriever::retrieveData(const KUrl &url)
{
    if (d->buffer)
        return;

    d->buffer = new QBuffer;
    d->buffer->open(QIODevice::WriteOnly);

    KUrl u = url;

    if (u.protocol() == "feed")
        u.setProtocol("http");

    d->job = KIO::get(u, !m_useCache, false);

    d->job->addMetaData("UserAgent", m_userAgent);


    QTimer::singleShot(1000*90, this, SLOT(slotTimeout()));

    connect(d->job, SIGNAL(data(KIO::Job*, const QByteArray&)),
            SLOT(slotData(KIO::Job*, const QByteArray&)));
    connect(d->job, SIGNAL(result(KIO::Job*)), SLOT(slotResult(KIO::Job*)));
    connect(d->job, SIGNAL(permanentRedirection(KIO::Job*, const KUrl&, const KUrl&)),
            SLOT(slotPermanentRedirection(KIO::Job*, const KUrl&, const KUrl&)));
}

void FileRetriever::slotTimeout()
{
    abort();

    delete d->buffer;
    d->buffer = NULL;

    d->lastError = KIO::ERR_SERVER_TIMEOUT;

    emit dataRetrieved(QByteArray(), false);
}

int FileRetriever::errorCode() const
{
    return d->lastError;
}

void FileRetriever::slotData(KIO::Job *, const QByteArray &data)
{
    d->buffer->write(data.data(), data.size());
}

void FileRetriever::slotResult(KIO::Job *job)
{
    QByteArray data = d->buffer->buffer();
    data.detach();

    delete d->buffer;
    d->buffer = NULL;

    d->lastError = job->error();
    emit dataRetrieved(data, d->lastError == 0);
}

void FileRetriever::slotPermanentRedirection(KIO::Job*, const KUrl&,
                                             const KUrl& newUrl)
{
    emit permanentRedirection(newUrl);
}

void FileRetriever::abort()
{
    if (d->job)
    {
        d->job->kill(true);
        d->job = NULL;
    }
}

struct OutputRetriever::OutputRetrieverPrivate
{
    OutputRetrieverPrivate() : process(0L), buffer(0L), lastError(0)
    {
    }

    ~OutputRetrieverPrivate()
    {
        delete process;
        delete buffer;
    }

    KShellProcess *process;
    QBuffer *buffer;
    int lastError;
};

OutputRetriever::OutputRetriever() : d(new OutputRetrieverPrivate)
{
}

OutputRetriever::~OutputRetriever()
{
    delete d;
}

void OutputRetriever::retrieveData(const KUrl &url)
{
   // Ignore subsequent calls if we didn't finish the previous job yet.
    if (d->buffer || d->process)
        return;

    d->buffer = new QBuffer;
    d->buffer->open(QIODevice::WriteOnly);

    d->process = new KShellProcess();
    connect(d->process, SIGNAL(processExited(KProcess*)),
            SLOT(slotExited(KProcess*)));
    connect(d->process, SIGNAL(receivedStdout(KProcess*, char*, int)),
            SLOT(slotOutput(KProcess*, char*, int)));
    *d->process << url.path();
    d->process->start(KProcess::NotifyOnExit, KProcess::Stdout);
}

int OutputRetriever::errorCode() const
{
    return d->lastError;
}

void OutputRetriever::slotOutput(KProcess *, char *data, int length)
{
    d->buffer->write(data, length);
}

void OutputRetriever::slotExited(KProcess *p)
{
    if (!p->normalExit())
        d->lastError = p->exitStatus();

    QByteArray data = d->buffer->buffer();
    data.detach();
    
    delete d->buffer;
    d->buffer = NULL;
    
    delete d->process;
    d->process = NULL;
    
    emit dataRetrieved(data, p->normalExit() && p->exitStatus() == 0);
}

} // namespace LibSyndication

#include "dataretriever.moc"
