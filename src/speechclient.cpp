/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>
    Copyright (C) 2009 Laurent Montel <montel@kde.org>

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

#include "speechclient.h"
#include <kstandarddirs.h>
#include "utils.h"

#include <krss/item.h>

#include <kcharsets.h>
#include <klocale.h>
#include <kdebug.h>
#include <k3staticdeleter.h>
#include <ktoolinvocation.h>
#include <kservicetypetrader.h>

#include "kspeechinterface.h"
#include <kspeech.h>

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


SpeechClient::SpeechClient() : QObject(), m_kspeech( 0 ), d(new SpeechClientPrivate)
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
  if ( !d->isTextSpeechInstalled )
    setupSpeechSystem();
  if ( text.isEmpty())
    return;
    uint jobNum = m_kspeech->say(text,0);
    d->pendingJobs.append(jobNum);
    if (d->pendingJobs.count() == 1)
    {
        emit signalJobsStarted();
        emit signalActivated(true);
    }
}

void SpeechClient::slotSpeak(const KRss::Item& item)
{
    if (!isTextToSpeechInstalled() || item.isNull())
        return;

    QString speakMe;
    speakMe += KCharsets::resolveEntities(Utils::stripTags(item.title()))
    + ". . . . "
    + KCharsets::resolveEntities(Utils::stripTags(item.content()));
    slotSpeak(speakMe, "en");
}

void SpeechClient::slotSpeak(const QList<KRss::Item>& items)
{
    if (!isTextToSpeechInstalled() || items.isEmpty())
        return;

    QString speakMe;

    Q_FOREACH( const KRss::Item& i, items)
    {
        if (!speakMe.isEmpty())
            speakMe += ". . . . . . " + i18n("Next Article: ");
        speakMe += KCharsets::resolveEntities(Utils::stripTags(i.title()))
        + ". . . . "
        + KCharsets::resolveEntities(Utils::stripTags(i.content()));
    }

    SpeechClient::self()->slotSpeak(speakMe, "en");
}

void SpeechClient::slotAbortJobs()
{
    if (!d->pendingJobs.isEmpty())
    {
      for (QList<uint>::ConstIterator it = d->pendingJobs.constBegin(); it != d->pendingJobs.constEnd(); ++it)
        {
          //removeText(*it);
        }

        d->pendingJobs.clear();
        emit signalJobsDone();
        emit signalActivated(false);
    }
}

void SpeechClient::textRemoved(const QString &appId, int jobNum, int state )
{
  if ( state == KSpeech::jsFinished || state == KSpeech::jsDeleted )
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
  if ( KStandardDirs::findExe( "kttsd" ).isEmpty() )
  {
    kDebug() <<"KTTSD not installed, disable support";
    d->isTextSpeechInstalled = false;
  }
  else
  {
    if ( QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kttsd") )
    {
      d->isTextSpeechInstalled = true;
    }
    else
    {
      QString error;

      if (KToolInvocation::startServiceByDesktopName("kttsd", QString(), &error) != 0)
      {
        kDebug() <<"Starting KTTSD failed with message" << error;
        d->isTextSpeechInstalled = false;
      }
      else
      {
        d->isTextSpeechInstalled = true;
      }
    }
    if (d->isTextSpeechInstalled)
    {
      if ( !m_kspeech )
      {
        m_kspeech = new org::kde::KSpeech("org.kde.kttsd", "/KSpeech", QDBusConnection::sessionBus());
        m_kspeech->setParent(this);
        m_kspeech->setApplicationName("Akregator Speech Text");
        connect(m_kspeech, SIGNAL(jobStateChanged(const QString&, int, int)),
                    this, SLOT(textRemoved(const QString&, int, int)));
        connect( QDBusConnection::sessionBus().interface(), SIGNAL( serviceUnregistered( const QString & ) ), this, SLOT( slotServiceUnregistered( const QString & ) ) );
        connect( QDBusConnection::sessionBus().interface(), SIGNAL( serviceOwnerChanged( const QString &, const QString &, const QString & ) ), this, SLOT( slotServiceOwnerChanged( const QString &, const QString &, const QString & ) ) );

      }
    }
  }
}

void SpeechClient::slotServiceUnregistered( const QString &service )
{
  if ( service == QLatin1String( "org.kde.kttsd" ) )
  {
    removeSpeech();
  }

}

void SpeechClient::slotServiceOwnerChanged( const QString &service, const QString &, const QString &newOwner )
{
  if ( service == QLatin1String( "org.kde.kttsd" ) && newOwner.isEmpty() )
  {
    removeSpeech();
  }
}

void SpeechClient::removeSpeech()
{
  d->isTextSpeechInstalled = false;
  disconnect( QDBusConnection::sessionBus().interface(), 0, this, 0 );

  delete m_kspeech;
  m_kspeech = 0;

}

} // namespace Akregator

#include "speechclient.moc"
