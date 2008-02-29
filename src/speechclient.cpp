/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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
#include "speechclient.h"
#include "utils.h"

#include <dcopclient.h>
#include <kcharsets.h>
#include <klocale.h>
#include <kdebug.h>
#include <k3staticdeleter.h>
#include <ktoolinvocation.h>
#include <kservicetypetrader.h>

#include <QString>

namespace Akregator 
{

class SpeechClient::SpeechClientPrivate
{
    public:

    bool isTextSpeechInstalled;
    QList<uint> pendingJobs;
};

SpeechClient* SpeechClient::m_self = 0;

static K3StaticDeleter<SpeechClient> speechclsd;

SpeechClient* SpeechClient::self()
{
    if (!m_self)
        m_self = speechclsd.setObject(m_self, new SpeechClient);
    return m_self;
}


SpeechClient::SpeechClient() : DCOPStub("kttsd", "KSpeech"), DCOPObject("akregatorpart_kspeechsink"), QObject(), d(new SpeechClientPrivate)
{
    d->isTextSpeechInstalled = false;
    setupSpeechSystem();
}

SpeechClient::~SpeechClient()
{
    delete d;
    d = 0;
}

void SpeechClient::slotSpeak(const QString& text, const QString& language)
{
    if (!isTextToSpeechInstalled() || text.isEmpty())
        return;
    uint jobNum = setText(text, language);
    startText(jobNum);
    d->pendingJobs.append(jobNum);
    if (d->pendingJobs.count() == 1)
    {
        emit signalJobsStarted();
        emit signalActivated(true);
    }
}

void SpeechClient::slotSpeak(const Article& article)
{
    if (!isTextToSpeechInstalled() || article.isNull())
        return;
    
    QString speakMe;
    speakMe += KCharsets::resolveEntities(Utils::stripTags((article).title())) 
    + ". . . . " 
    + KCharsets::resolveEntities(Utils::stripTags((article).description()));
    slotSpeak(speakMe, "en");
}

void SpeechClient::slotSpeak(const QList<Article>& articles)
{
    if (!isTextToSpeechInstalled() || articles.isEmpty())
        return;

    QString speakMe;

    for (QList<Article>::ConstIterator it = articles.begin(); it != articles.end(); ++it)
    {
        if (!speakMe.isEmpty())
            speakMe += ". . . . . . " + i18n("Next Article: ");
        speakMe += KCharsets::resolveEntities(Utils::stripTags((*it).title())) 
        + ". . . . " 
        + KCharsets::resolveEntities(Utils::stripTags((*it).description()));
    }

    SpeechClient::self()->slotSpeak(speakMe, "en");
}

void SpeechClient::slotAbortJobs()
{
    if (!d->pendingJobs.isEmpty())
    {
        for (QList<uint>::ConstIterator it = d->pendingJobs.begin(); it != d->pendingJobs.end(); ++it)
        {
            removeText(*it);
        }

        d->pendingJobs.clear();
        emit signalJobsDone();
        emit signalActivated(false);
    }
}

ASYNC SpeechClient::textRemoved(const DCOPCString& /*appId*/, uint jobNum)
{
    kDebug() <<"SpeechClient::textRemoved() called";
    if (d->pendingJobs.contains(jobNum))
    {
        d->pendingJobs.removeAll(jobNum);
        if (d->pendingJobs.isEmpty())
        {
            emit signalJobsDone();
            emit signalActivated(false);
        }
    }
}

bool SpeechClient::isTextToSpeechInstalled() const
{
    return d->isTextSpeechInstalled;
}

void SpeechClient::setupSpeechSystem()
{
    KService::List offers = KServiceTypeTrader::self()->query("DBUS/Text-to-Speech", "Name == 'KTTSD'");
    if (offers.isEmpty())
    {
        kDebug() <<"KTTSD not installed, disable support";
        d->isTextSpeechInstalled = false;
    }
    else
    {
        DCOPClient* client = dcopClient();
        //client->attach();
        if (client->isApplicationRegistered("kttsd")) 
        {
            d->isTextSpeechInstalled = true;
        }
        else
        {
            QString error;
            if (KToolInvocation::startServiceByDesktopName("kttsd", QStringList(), &error))
            {
                kDebug() <<"Starting KTTSD failed with message" << error;
                d->isTextSpeechInstalled = false;
            }
            else
            {
                d->isTextSpeechInstalled = true;
            }
        }
    }
    if (d->isTextSpeechInstalled)
    {

        bool c = connectDCOPSignal("kttsd", "KSpeech",
                "textRemoved(QCString, uint)",
                "textRemoved(QCString, uint)",
                false);
        if (!c)
            kDebug() <<"SpeechClient::setupSpeechSystem(): connecting signals failed";
        c = connectDCOPSignal("kttsd", "KSpeech",
                "textFinished(QCString, uint)",
                "textRemoved(QCString, uint)",
                false);
    }
}


} // namespace Akregator

#include "speechclient.moc"
