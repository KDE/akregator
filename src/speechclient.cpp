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
#include "article.h"
#include "utils.h"

#include <kcharsets.h>
#include <klocale.h>
#include <qdebug.h>
#include <ktoolinvocation.h>
#include <kservicetypetrader.h>

#include "kspeechinterface.h"
#include <QString>
#include <kspeech.h>
namespace Akregator
{

class SpeechClient::SpeechClientPrivate
{
    public:

    bool isTextSpeechInstalled;
    QList<uint> pendingJobs;
};

SpeechClient* SpeechClient::m_self = 0;

SpeechClient* SpeechClient::self()
{
    static SpeechClient sself;
    if (!m_self)
        m_self = &sself;
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

void SpeechClient::slotSpeak(const QString& text, const QString& /*language*/)
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
  qDebug()<<" SpeechClient::slotSpeak(const Articlessssssssssss& article) :"<<articles.isEmpty()<<" isTextToSpeechInstalled :"<<isTextToSpeechInstalled();

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
      for (QList<uint>::ConstIterator it = d->pendingJobs.constBegin(); it != d->pendingJobs.constEnd(); ++it)
        {
          //removeText(*it);
        }

        d->pendingJobs.clear();
        emit signalJobsDone();
        emit signalActivated(false);
    }
}

void SpeechClient::textRemoved(const QString &/*appId*/, int jobNum, int state )
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
  if ( QStandardPaths::findExecutable( "kttsd" ).isEmpty() )
  {
    qDebug() <<"KTTSD not installed, disable support";
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
        qDebug() <<"Starting KTTSD failed with message" << error;
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
        connect(m_kspeech, SIGNAL(jobStateChanged(QString,int,int)),
                    this, SLOT(textRemoved(QString,int,int)));
        connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceUnregistered(QString)), this, SLOT(slotServiceUnregistered(QString)) );
        connect( QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)), this, SLOT(slotServiceOwnerChanged(QString,QString,QString)) );

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

