/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>
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
#include "feed.h"
#include "feedstorage.h"
#include "storage.h"
#include "librss/librss.h"
#include "shared.h"
#include "utils.h"

#include <qdatetime.h>
#include <qdom.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <krfcdate.h>
#include <kdebug.h>
#include <kurl.h>


namespace Akregator {

struct Article::Private : public Shared
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
    QString guid;
    uint hash;
    Backend::FeedStorage* archive;
    QDateTime pubDate;
    Feed* feed;
};

Article::Article() : d(new Private)
{
    d->hash = 0;
    d->status = 0;
    d->feed = 0;
    d->archive = 0;
    d->pubDate.setTime_t(1);

}

Article::Article(const QString& guid, Feed* feed) : d(new Private)
{
    d->feed = feed;
    d->guid = guid;
    d->archive = Backend::Storage::getInstance()->archiveFor(feed->xmlUrl());
    d->status = d->archive->status(d->guid);
    d->pubDate.setTime_t(d->archive->pubDate(d->guid));
    d->hash = d->archive->hash(d->guid);
}

void Article::initialize(RSS::Article article, Backend::FeedStorage* archive)
{
    d->archive = archive;
    d->status = Private::New;
    d->hash = Utils::calcHash(article.title() + article.description() + article.link().url() + article.commentsLink().url() );

    d->guid = article.guid();
    
    if (!d->archive->contains(d->guid))
    {
        d->archive->addEntry(d->guid);

        if (article.meta("deleted") == "true") 
        { // if article is in deleted state, we just add the status and omit the rest
            d->status = Private::Read | Private::Deleted;
            d->archive->setStatus(d->guid, d->status);
        }
        else
        { // article is not deleted, let's add it to the archive
        
            d->archive->setHash(d->guid, d->hash);
            QString title = article.title().isEmpty() ? buildTitle(article.description()) :  article.title();
            d->archive->setTitle(d->guid, title);
            d->archive->setDescription(d->guid, article.description());
            d->archive->setLink(d->guid, article.link().url());
            d->archive->setComments(d->guid, article.comments());
            d->archive->setCommentsLink(d->guid, article.commentsLink().url());
            d->archive->setGuidIsPermaLink(d->guid, article.guidIsPermaLink());
            d->archive->setGuidIsHash(d->guid, article.meta("guidIsHash") == "true");
            d->pubDate = article.pubDate().isValid() ? article.pubDate() : QDateTime::currentDateTime();
            d->archive->setPubDate(d->guid, d->pubDate.toTime_t());

            QValueList<RSS::Category> cats = article.categories();
            QValueList<RSS::Category>::ConstIterator end = cats.end();

            for (QValueList<RSS::Category>::ConstIterator it = cats.begin(); it != end; ++it)
            {
                Backend::Category cat;

                cat.term = (*it).category();
                cat.scheme = (*it).domain();
                cat.name = (*it).category();

                d->archive->addCategory(d->guid, cat);
            }

            if (!article.enclosure().isNull())
            {
                d->archive->setEnclosure(d->guid, article.enclosure().url(), article.enclosure().type(), article.enclosure().length());
            }
            else
            {
                d->archive->removeEnclosure(d->guid);
            }

            QString status = article.meta("status");
            
            if (!status.isEmpty())
            {
                int statusInt = status.toInt();
                if (statusInt == New)
                    statusInt = Unread;
                setStatus(statusInt);
            }
            setKeep(article.meta("keep") == "true");
        }
    }
    else
    {
        // always update comments count, as it's not used for hash calculation
        d->archive->setComments(d->guid, article.comments());
        if (d->hash != d->archive->hash(d->guid)) //article is in archive, was it modified?
        { // if yes, update
            d->pubDate.setTime_t(d->archive->pubDate(d->guid));
            d->archive->setHash(d->guid, d->hash);
            QString title = article.title().isEmpty() ? buildTitle(article.description()) :  article.title();
            d->archive->setTitle(d->guid, title);
            d->archive->setDescription(d->guid, article.description());
            d->archive->setLink(d->guid, article.link().url());
            d->archive->setCommentsLink(d->guid, article.commentsLink().url());
        }
    }
}

Article::Article(RSS::Article article, Feed* feed) : d(new Private)
{
    //assert(feed)
    d->feed = feed;
    initialize(article, Backend::Storage::getInstance()->archiveFor(feed->xmlUrl()));
}

Article::Article(RSS::Article article, Backend::FeedStorage* archive) : d(new Private)
{
    d->feed = 0;
    initialize(article, archive);
}

bool Article::isNull() const
{
    return d->archive == 0; // TODO: use proper null state
}

void Article::offsetPubDate(int secs)
{
   d->pubDate = d->pubDate.addSecs(secs);
   d->archive->setPubDate(d->guid, d->pubDate.toTime_t());

}

void Article::setDeleted()
{
    if (isDeleted())
        return;
  
    setStatus(Read);
    d->status = Private::Deleted | Private::Read;
    d->archive->setStatus(d->guid, d->status);
    d->archive->setDeleted(d->guid);

    if (d->feed)
        d->feed->setArticleDeleted(*this);
}

bool Article::isDeleted() const
{
    return (d->status & Private::Deleted) != 0;
}

Article::Article(const Article &other) : d(new Private)
{
    *this = other;
}

Article::~Article()
{
    if (d->deref())
    {
        delete d;
        d = 0;
    }
}

Article &Article::operator=(const Article &other)
{
    if (this != &other) {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}


bool Article::operator<(const Article &other) const
{
    return pubDate() > other.pubDate() || 
            (pubDate() == other.pubDate() && guid() < other.guid() );
}

bool Article::operator<=(const Article &other) const
{
    return (pubDate() > other.pubDate() || *this == other);
}

bool Article::operator>(const Article &other) const
{
    return pubDate() < other.pubDate() || 
            (pubDate() == other.pubDate() && guid() > other.guid() );
}

bool Article::operator>=(const Article &other) const
{
    return (pubDate() > other.pubDate() || *this == other);
}

bool Article::operator==(const Article &other) const
{
    return d->guid == other.guid();
}

int Article::status() const
{
    if ((d->status & Private::Read) != 0)
        return Read;

    if ((d->status & Private::New) != 0)
        return New;
    else
        return Unread;
}

void Article::setStatus(int stat)
{
    int oldStatus = status();

    if (oldStatus != stat)
    {
        switch (stat)
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
        d->archive->setStatus(d->guid, d->status);
        if (d->feed)
            d->feed->setArticleChanged(*this, oldStatus);
     }
}

QString Article::title() const
{
    return d->archive->title(d->guid);
}

KURL Article::link() const
{
    return d->archive->link(d->guid);
}

QString Article::description() const
{
    return d->archive->description(d->guid);
}

QString Article::guid() const
{
    return d->guid;
}

KURL Article::commentsLink() const
{
    return d->archive->commentsLink(d->guid);
}


int Article::comments() const
{
    
    return d->archive->comments(d->guid);
}


bool Article::guidIsPermaLink() const
{
    return d->archive->guidIsPermaLink(d->guid);
}

bool Article::guidIsHash() const
{
    return d->archive->guidIsHash(d->guid);
}

uint Article::hash() const
{
    return d->hash;
}

bool Article::keep() const
{
    return (d->status & Private::Keep) != 0;
}

RSS::Enclosure Article::enclosure() const
{
    bool hasEnc;
    QString url, type;
    int length;
    d->archive->enclosure(d->guid, hasEnc, url, type, length);
    return hasEnc ? RSS::Enclosure(url, length, type) : RSS::Enclosure();

    
}


void Article::setKeep(bool keep)
{
    d->status = keep ? (d->status | Private::Keep) : (d->status & ~Private::Keep);
    d->archive->setStatus(d->guid, d->status);
    if (d->feed)
        d->feed->setArticleChanged(*this);
}

void Article::addTag(const QString& tag)
{
    d->archive->addTag(d->guid, tag);
    if (d->feed)
        d->feed->setArticleChanged(*this);
}

void Article::removeTag(const QString& tag)
{
    d->archive->removeTag(d->guid, tag);
    if (d->feed)
        d->feed->setArticleChanged(*this);
}

bool Article::hasTag(const QString& tag) const
{
    return d->archive->tags(d->guid).contains(tag);
}

QStringList Article::tags() const
{
    return d->archive->tags(d->guid);
}
            
Feed* Article::feed() const
{ return d->feed; }

const QDateTime& Article::pubDate() const
{
    return d->pubDate;
}

QString Article::buildTitle(const QString& description)
{
    QString s = description;
    if (description.stripWhiteSpace().isEmpty())
        return "";
        
    int i = s.find('>',500); /*avoid processing too much */
    if (i != -1)
        s = s.left(i+1);
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
} // namespace Akregator
