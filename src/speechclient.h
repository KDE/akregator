/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef AKREGATOR_SPEECHCLIENT_H
#define AKREGATOR_SPEECHCLIENT_H

#include <kspeechsink.h>
#include "kspeech_stub.h"

#include <qobject.h>
#include <QList>
#include <Q3CString>

class QString;
template <class T> class QList;

namespace Akregator
{

class Article;

class SpeechClient : public QObject, public KSpeech_stub, virtual public KSpeechSink
{

    Q_OBJECT

    public:

        static SpeechClient* self();
        bool isTextToSpeechInstalled() const;
        virtual ~SpeechClient();

    public slots:

        void slotSpeak(const QString& text, const QString& language);
        void slotSpeak(const Article& article);
        void slotSpeak(const QList<Article>& articles);
        void slotAbortJobs();

    signals:
      
        /** emitted when the job queue was empty before and the first job was just queued */
        void signalJobsStarted();

        /** emitted when all jobs were finished or aborted and no further jobs are queued */
        void signalJobsDone();

        void signalActivated(bool);

    protected:

        SpeechClient();
        void setupSpeechSystem();

        ASYNC textRemoved(const DCOPCString& appId, uint jobNum);

    private:
     
        class SpeechClientPrivate;
        SpeechClientPrivate* d;
        
        static SpeechClient* m_self;
};

} // namespace Akregator

#endif // AKREGATOR_SPEECHCLIENT_H
