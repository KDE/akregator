/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "myarticle.h"
#include "librss/tools_p.h"
#include <qdatetime.h>
#include <kurl.h>

using namespace Akregator;
using namespace RSS;

struct MyArticle::Private : public RSS::Shared
{
    Article article;
    QDateTime fetchDate;
};

MyArticle::MyArticle() : d(new Private)
{
}

MyArticle::MyArticle(Article article) : d(new Private)
{
    d->article = article;
    d->fetchDate = QDateTime::currentDateTime();
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

   if (pubDate().isValid() && (pubDate() == other.pubDate()))
      return true;

   // FIXME it shouldn't be _that_ strict checking, should it?
   return d->article   == other.d->article
       && d->fetchDate == other.d->fetchDate;
}

QString MyArticle::title() const
{
    return d->article.title();
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

