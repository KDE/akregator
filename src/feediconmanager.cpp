/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "feediconmanager.h"

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include <qpixmap.h>


using namespace Akregator;

FeedIconManager *FeedIconManager::m_instance = 0;

FeedIconManager *FeedIconManager::self()
{
    if (!m_instance)
        m_instance = new FeedIconManager();

    return m_instance;
}

FeedIconManager::FeedIconManager(QObject * parent, const char *name)
:  QObject(parent, name), DCOPObject("FeedIconManager")
{
    connectDCOPSignal("kded",
                      "favicons", "iconChanged(bool, QString, QString)",
                      "slotIconChanged(bool, QString, QString)", false);
}

FeedIconManager::~FeedIconManager()
{
    delete m_instance;
}

void FeedIconManager::loadIcon(const QString & url)
{
    if (url.isEmpty()) {
        return;
    }

    KURL u(url);

    QString iconFile = iconLocation(u);
    if (iconFile.isNull()) {
        QByteArray data;
        QDataStream ds(data, IO_WriteOnly);
        ds << u;
        kapp->dcopClient()->send("kded", "favicons", "downloadHostIcon(KURL)",
                                 data);
    } else {
        emit iconChanged(url,
                         QPixmap(KGlobal::dirs()->
                                 findResource("cache", iconFile+".png")));
    }
}

QString FeedIconManager::iconLocation(const KURL & url) const
{
    QByteArray data, reply;
    QCString replyType;
    QDataStream ds(data, IO_WriteOnly);

    ds << url;

    kapp->dcopClient()->call("kded", "favicons", "iconForURL(KURL)", data,
                             replyType, reply);

    if (replyType == "QString") {
        QDataStream replyStream(reply, IO_ReadOnly);
        QString result;
        replyStream >> result;
        return result;
    }

    return QString::null;
}

void FeedIconManager::slotIconChanged(bool /*isHost*/, const QString& hostOrURL,
                                  const QString& iconName)
{
    emit iconChanged(hostOrURL,
                     QPixmap(KGlobal::dirs()->
                             findResource("cache", iconName+".png")));
}


#include "feediconmanager.moc"
