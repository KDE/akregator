/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "myarticle.h"
#include "librss/tools_p.h"

#include <qdatetime.h>
#include <qregexp.h>

#include <krfcdate.h>
#include <kurl.h>

using namespace Akregator;
using namespace RSS;

struct MyArticle::Private : public RSS::Shared
{
    Article article;
    QDateTime fetchDate;
    QString title;
    int status;
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

bool MyArticle::operator==(const MyArticle &other) const
{
   // play tricky here
   // we return true if guids match, even if texts differ => helps ArticleSequence
   // we return true if links match, ditto
   // otherwise we check for complete equivalence

   if (!d->article.guid().isEmpty() && (d->article.guid() == other.d->article.guid()))
      return true;

   if (d->article.link().isValid() && (d->article.link() == other.d->article.link()))
      return true;

   if (d->article.pubDate().isValid() && (d->article.pubDate() == other.d->article.pubDate()))
           return true;
   //if (pubDate().isValid() && (pubDate() == other.pubDate()))
   //   return true;

   // FIXME it shouldn't be _that_ strict checking, should it?
   return d->article   == other.d->article
       && d->fetchDate == other.d->fetchDate;
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

bool MyArticle::guidIsPermaLink() const
{
    return d->article.guidIsPermaLink();
}

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
        QDomText gt=doc.createTextNode(link().url());
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

    QDomElement metanode = doc.createElement( "metaInfo:meta" );
    
    QDomElement statnode = doc.createElement( "item" );
    statnode.setAttribute("type","status");
    
    QDomText stat=doc.createTextNode(QString::number(d->status));
    statnode.appendChild(stat);
    metanode.appendChild(statnode);
    parent.appendChild(metanode);
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
