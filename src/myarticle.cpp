/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#include "myarticle.h"
#include "feed.h"
#include "librss/tools_p.h"

#include <qdatetime.h>
#include <qdom.h>
#include <qregexp.h>

#include <krfcdate.h>
#include <kdebug.h>
#include <kurl.h>

using namespace Akregator;
using namespace RSS;

struct MyArticle::Private : public RSS::Shared
{
    /** The status of the article is stored in an int, the bits having the
        following meaning:

        0000 0001 Deleted
        0000 0010 Trash
        0000 0100 New
        0000 1000 Read
        0001 0000 Keep
     */

    enum Status {Deleted=0x01, Trash=0x02, New=0x04, Read=0x08, Keep=0x10};
    int status;

    bool guidIsHash;
    uint hash;
    Article article;
    QDateTime fetchDate;
    QString title;

    Feed* feed;
};

MyArticle::MyArticle() : d(new Private)
{
    d->hash = 0;
    d->status = 0;
    d->feed = 0;
    d->guidIsHash = false;
}

MyArticle::MyArticle(Article article) : d(new Private)
{
    d->hash = 0;
    d->status = 0;
    d->article = article;
    d->feed = 0;
    d->fetchDate = QDateTime::currentDateTime();

    if (article.title().isEmpty())
        d->title=buildTitle();
    else
        d->title=article.title();

    QString status = d->article.meta("status");

    if (!status.isEmpty())
        setStatus(status.toInt());

    setKeep((article.meta("keep") == "true"));
    if (article.meta("deleted") == "true")
        setDeleted();
    d->guidIsHash = (article.meta("guidIsHash") == "true");

    if (!d->guidIsHash)
    {
        QString hashStr = article.meta("hash");

        bool parsedOk = false;
        uint parsed = hashStr.toUInt(&parsedOk, 16);
        if (!parsedOk)
        {
            d->hash = calcHash(title() + description() + link().url() + commentsLink().url()
                    + QString::number(comments()) );
        }
        else
            d->hash = parsed;
    }
}

void MyArticle::setDeleted()
{
    d->status = Private::Deleted | Private::Read;
}

bool MyArticle::isDeleted() const
{
    return (d->status & Private::Deleted) != 0;
}

MyArticle::MyArticle(const MyArticle &other) : d(new Private)
{
    *this = other;
}

MyArticle::~MyArticle()
{
    if (d->deref())
        delete d;
}

MyArticle &MyArticle::operator=(const MyArticle &other)
{
    if (this != &other) {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}

void MyArticle::offsetFetchTime(int secs)
{
    d->fetchDate=d->fetchDate.addSecs(secs);
}

bool MyArticle::operator<(const MyArticle &other) const
{
    return pubDate() > other.pubDate();
}

bool MyArticle::operator<=(const MyArticle &other) const
{
    return pubDate() >= other.pubDate();
}

bool MyArticle::operator>(const MyArticle &other) const
{
    return pubDate() < other.pubDate();
}

bool MyArticle::operator>=(const MyArticle &other) const
{
    return pubDate() <= other.pubDate();
}

bool MyArticle::operator==(const MyArticle &other) const
{
    return (d->article.guid() == other.d->article.guid());
}

int MyArticle::status() const
{
    if ((d->status & Private::Read) != 0)
        return Read;

    if ((d->status & Private::New) != 0)
        return New;
    else
        return Unread;
}

void MyArticle::setStatus(int status)
{
    switch (status)
    {
        case Read:
            d->status = (d->status | Private::Read) & ~Private::New;
            break;
        case Unread:
            d->status = (d->status & ~Private::Read) & ~Private::New;
            break;
        case New:
            d->status = (d->status | Private::New) & ~Private::Read;
            break;
    }
}

QString MyArticle::title() const
{
    return d->title.remove("\\");
}

const KURL &MyArticle::link() const
{
    return d->article.link();
}

QString MyArticle::description() const
{
    return d->article.description();
}

QString MyArticle::guid() const
{
    return d->article.guid();
}

const KURL &MyArticle::commentsLink() const
{
    return d->article.commentsLink();
}


int MyArticle::comments() const
{
    return d->article.comments();
}


bool MyArticle::guidIsPermaLink() const
{
    return d->article.guidIsPermaLink();
}

/* taken from some website... -fo
* djb2
* This algorithm was first reported by Dan Bernstein
* many years ago in comp.lang.c
*/
uint MyArticle::calcHash(const QString& str)
{
    const char* s = str.ascii();
    uint hash = 5381;
    int c;
    while ( ( c = *s++ ) ) hash = ((hash << 5) + hash) + c; // hash*33 + c
    return hash;
}

bool MyArticle::guidIsHash() const
{
    return d->guidIsHash;
}

uint MyArticle::hash() const
{
    return d->hash;
}

bool MyArticle::keep() const
{
    return (d->status & Private::Keep) != 0;
}

void MyArticle::setKeep(bool keep)
{
    d->status = keep ? (d->status | Private::Keep) : (d->status & ~Private::Keep);
}

void MyArticle::setFeed(Feed* feed)
{
    d->feed = feed;
}
Feed* MyArticle::feed() const
{ return d->feed; }

const QDateTime &MyArticle::pubDate() const
{
    return d->article.pubDate().isValid() ? d->article.pubDate()
                                          : d->fetchDate;
}

KURLLabel *MyArticle::widget(QWidget *parent, const char *name) const
{
    return d->article.widget(parent, name);
}

void MyArticle::dumpXmlData( QDomElement parent, QDomDocument doc ) const
{

    if (!guid().isEmpty())
    {
        QDomElement gnode = doc.createElement( "guid" );
        gnode.setAttribute("isPermaLink",guidIsPermaLink()?"true":"false");
        QDomText gt=doc.createTextNode(guid());
        gnode.appendChild(gt);
        parent.appendChild(gnode);
    }

    if (pubDate().isValid())
    {
        QDomElement pnode = doc.createElement( "pubDate" );
        QDomText dat=doc.createTextNode(KRFCDate::rfc2822DateString(pubDate().toTime_t()));
        pnode.appendChild(dat);
        parent.appendChild(pnode);
    }

    if ( isDeleted() )
    {
        QDomElement metanode = doc.createElement( "metaInfo:meta" );
        metanode.setAttribute("type","deleted");
        QDomText stat=doc.createTextNode( "true" );
        metanode.appendChild(stat);
        parent.appendChild(metanode);
    }
    else
    {
        if ( !title().isEmpty() )
        {
            QDomElement tnode = doc.createElement( "title" );
            QDomText t = doc.createTextNode( title() );
            tnode.appendChild(t);
            parent.appendChild(tnode);
        }

        if (link().isValid())
        {
            QDomElement lnode = doc.createElement( "link" );
            QDomText ht=doc.createTextNode(link().url());
            lnode.appendChild(ht);
            parent.appendChild(lnode);
        }

        if (!description().isEmpty())
        {
            QDomElement snode = doc.createElement( "description" );
            QDomCDATASection dt=doc.createCDATASection( description() );
            snode.appendChild(dt);
            parent.appendChild(snode);
        }

        if (commentsLink().isValid())
        {
            QDomElement lnode = doc.createElement( "wfw:comment" );
            QDomText ht=doc.createTextNode(commentsLink().url());
            lnode.appendChild(ht);
            parent.appendChild(lnode);
        }

        if (comments())
        {
            QDomElement lnode = doc.createElement( "slash:comments" );
            QDomText ht=doc.createTextNode(QString::number(comments()));
            lnode.appendChild(ht);
            parent.appendChild(lnode);
        }

        QDomElement metanode = doc.createElement( "metaInfo:meta" );
        metanode.setAttribute("type","status");
        QDomText stat=doc.createTextNode( QString::number(status()) );
        metanode.appendChild(stat);
        parent.appendChild(metanode);


        if (guidIsHash())
        {
            metanode = doc.createElement( "metaInfo:meta" );
            metanode.setAttribute("type", "guidIsHash");
            metanode.appendChild(doc.createTextNode("true"));
            parent.appendChild(metanode);
        }
        else
        {
            metanode = doc.createElement( "metaInfo:meta" );
            metanode.setAttribute("type", "hash");
            metanode.appendChild(doc.createTextNode(QString::number(hash(), 16)));
            parent.appendChild(metanode);
        }

        if (keep())
        {
            metanode = doc.createElement( "metaInfo:meta" );
            metanode.setAttribute("type", "keep");
            metanode.appendChild(doc.createTextNode("true"));
            parent.appendChild(metanode);
        }
    } // if not deleted

}


QString MyArticle::buildTitle()
{
    QString s=d->article.description();
    int i=s.find('>',500); /*avoid processing too much */
    if (i != -1)
        s=s.left(i+1);
    QRegExp rx("(<([^\\s>]*)(?:[^>]*)>)[^<]*", false);
    QString tagName, toReplace, replaceWith;
    while (rx.search(s) != -1 )
    {
        tagName=rx.cap(2);
        if (tagName=="SCRIPT"||tagName=="script")
            toReplace=rx.cap(0); // strip tag AND tag contents
        else if (tagName.startsWith("br") || tagName.startsWith("BR"))
        {
            toReplace=rx.cap(1);
            replaceWith=" ";
        }
        else
            toReplace=rx.cap(1);  // strip just tag
        s=s.replace(s.find(toReplace),toReplace.length(),replaceWith); // do the deed
    }
    if (s.length()> 90)
        s=s.left(90)+"...";
    return s.simplifyWhiteSpace();
}
