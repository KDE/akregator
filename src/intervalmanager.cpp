/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

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
            if (!url.isNull())
                m_siteMap[url] = e.attribute("lastFetched","0").toUInt();
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
