/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "intervalmanager.h"

#include <qdom.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kstandarddirs.h>
#include <kconfig.h>

using namespace Akregator;

IntervalManager *IntervalManager::m_instance = 0;

IntervalManager *IntervalManager::self()
{
    if (!m_instance)
    {
        m_instance = new IntervalManager();
    }
    return m_instance;
}

IntervalManager::IntervalManager(QObject * parent, const char *name)
:  QObject(parent, name)
{
    m_syncTimer=new QTimer(this);
    connect(m_syncTimer, SIGNAL(timeout()), this, SLOT(sync()));
    read();
}

IntervalManager::~IntervalManager()
{
    delete m_instance;
}

void IntervalManager::read()
{
    QString filePath=KGlobal::dirs()->saveLocation("data", "akregator")+"/sites.xml";
    QFile file(filePath);

    if ( !file.open( IO_ReadOnly ) ) {
        return;
    }

    QTextStream stream( &file );
    stream.setEncoding(QTextStream::UnicodeUTF8);
    QString data=stream.read();
    QDomDocument doc;

    if (!doc.setContent(data))
    {
        return;
    }

    QDomElement root = doc.documentElement();

    if (root.tagName().lower() != "sites")
        return;

    QDomNode n = root.firstChild();
    while( !n.isNull() )
    {
        QDomElement e = n.toElement();
        if( !e.isNull() && e.tagName()== "site")
        {
            QString url=e.attribute("url");
            if (url.isNull())
                m_siteMap[url]=e.attribute("lastFetched","0").toUInt();
        }
        n = n.nextSibling();
    }
}

void IntervalManager::sync()
{
    QString filePath=KGlobal::dirs()->saveLocation("data", "akregator")+"/sites.xml";
    QFile file(filePath);

    if ( !file.open( IO_WriteOnly ) ) {
        kdDebug() << "load error"<<endl;
        return;
    }

    QTextStream stream( &file );
    stream.setEncoding(QTextStream::UnicodeUTF8);

    QDomDocument doc;
    QDomProcessingInstruction z = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild( z );

    QDomElement root = doc.createElement( "sites" );
    doc.appendChild( root );
    
    QMap<QString, int>::Iterator it;
    for (it=m_siteMap.begin(); it != m_siteMap.end(); ++it)
    {
        QDomElement e = doc.createElement( "site" );
        e.setAttribute("url", it.key());
        e.setAttribute("lastFetched", it.data());
        root.appendChild(e);
    }   
    stream<<doc.toString();
}


void IntervalManager::feedFetched(const QString& url)
{
    QDateTime dt=QDateTime::currentDateTime();
    m_siteMap.insert(url, dt.toTime_t());
    if (!m_syncTimer->isActive())
    {
        m_syncTimer->start(10000, true);
    }
}

uint IntervalManager::lastFetchTime(const QString& url)
{
    return m_siteMap[url];
}

#include "intervalmanager.moc"

// vim: set et ts=4 sts=4 sw=4:
