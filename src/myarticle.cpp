/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

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
    Article article;
    QDateTime fetchDate;
    QString title;
    int status;
    bool keep;
    Feed* feed;
};

MyArticle::MyArticle() : d(new Private)
{
}

MyArticle::MyArticle(Article article) : d(new Private)
{
    d->article = article;
    d->fetchDate = QDateTime::currentDateTime();
    
    if (article.title().isEmpty())
        d->title=buildTitle();
    else
        d->title=article.title();
    d->status=d->article.meta("status").toInt();
    
    d->keep = article.meta("keep") == "true" ? true : false;
    
    if (d->keep)
        kdDebug() << "KEEP" << d->title << endl;
    d->feed = 0;
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

bool MyArticle::operator>(const MyArticle &other) const
{
    return pubDate() < other.pubDate();
}

bool MyArticle::operator==(const MyArticle &other) const
{
    return (d->article.guid() == other.d->article.guid());
}

int MyArticle::status() const
{
    return d->status;
}

void MyArticle::setStatus(int status)
{
    d->status=status;
}

QString MyArticle::title() const
{
    return d->title;
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

bool MyArticle::keep() const
{
    return d->keep;
}

void MyArticle::setKeep(bool keep)
{
    d->keep = keep;
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
    QDomElement tnode = doc.createElement( "title" );
    QDomText t=doc.createTextNode( title() );
    tnode.appendChild(t);
    parent.appendChild(tnode);

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
    QDomText stat=doc.createTextNode(QString::number(d->status));
    metanode.appendChild(stat);
    parent.appendChild(metanode);
    
    if ( d->keep )
    {
//        metanode = doc.createElement( "metaInfo:meta" );
        metanode.setAttribute("type", "keep");
        metanode.appendChild(doc.createTextNode( d->keep ? "true" : "false"));
        parent.appendChild(metanode);
    }
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
