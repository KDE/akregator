/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>
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


#define CRASH_CHECKS


#include "article.h"
#include "feed.h"
#include "feedstorage.h"
#include "shared.h"
#include "storage.h"
#include "utils.h"

#include <syndication/syndication.h>

#include <QDateTime>
#include <qdom.h>
#include <QRegExp>
#include <QList>

#include <kdebug.h>
#include <kurl.h>
#ifdef CRASH_CHECKS
#include <kmessagebox.h>
#endif // CRASH_CHECKS

#include <cassert>

using namespace boost;
using namespace Syndication;

namespace {

QString buildTitle(const QString& description)
{
    QString s = description;
    if (description.trimmed().isEmpty())
        return "";

    int i = s.indexOf('>',500); /*avoid processing too much */
    if (i != -1)
        s = s.left(i+1);
    QRegExp rx("(<([^\\s>]*)(?:[^>]*)>)[^<]*", Qt::CaseInsensitive);
    QString tagName, toReplace, replaceWith;
    while (rx.indexIn(s) != -1 )
    {
        tagName=rx.cap(2);
        if (tagName=="SCRIPT"||tagName=="script")
            toReplace=rx.cap(0); // strip tag AND tag contents
        else if (tagName.startsWith(QLatin1String("br")) || tagName.startsWith(QLatin1String("BR")))
        {
            toReplace=rx.cap(1);
            replaceWith=" ";
        }
        else
            toReplace=rx.cap(1);  // strip just tag
        s=s.replace(s.indexOf(toReplace),toReplace.length(),replaceWith); // do the deed
    }
    if (s.length()> 90)
        s=s.left(90)+"...";
    return s.simplified();
}

}

namespace Akregator {

struct Article::Private : public Shared
{
    Private();
    Private( const QString& guid, Feed* feed, Backend::FeedStorage* archive );
    Private( const ItemPtr& article, Feed* feed, Backend::FeedStorage* archive );

    /** The status of the article is stored in an int, the bits having the
        following meaning:

        0000 0001 Deleted
        0000 0010 Trash
        0000 0100 New
        0000 1000 Read
        0001 0000 Keep
     */
    enum Status
    {
        Deleted=0x01,
        Trash=0x02,
        New=0x04,
        Read=0x08,
        Keep=0x10
    };

    Feed* feed;
    QString guid;
    Backend::FeedStorage* archive;
    int status;
    uint hash;
    QDateTime pubDate;
    mutable shared_ptr<const Enclosure> enclosure;
};

namespace {
    class EnclosureImpl : public Enclosure
    {
    public:
        EnclosureImpl( const QString& url, const QString& type, uint length ) : m_url( url ), m_type( type ), m_length( length ) {}
        /* reimp */ QString url() const { return m_url; }
        /* reimp */ QString type() const { return m_type; }
        /* reimp */ QString title() const { return m_title; }
        /* reimp */ uint length() const { return m_length; }
        /* reimp */ uint duration() const { return 0; }
        /* reimp */ bool isNull() const { return m_url.isNull(); }
    private:
        QString m_url;
        QString m_type;
        QString m_title;
        uint m_length;
    };
}

Article::Private::Private()
  : feed( 0 ),
    archive( 0 ),
    status( 0 ),
    hash( 0 ),
    pubDate( QDateTime::fromTime_t( 1 ) )
{
}

Article::Private::Private( const QString& guid_, Feed* feed_, Backend::FeedStorage* archive_ )
  : feed( feed_ ),
    guid( guid_ ),
    archive( archive_ ),
    status( archive->status( guid ) ),
    hash( archive->hash( guid ) ),
    pubDate( QDateTime::fromTime_t( archive->pubDate( guid ) ) )
{
}

Article::Private::Private( const ItemPtr& article, Feed* feed_, Backend::FeedStorage* archive_ )
  : feed( feed_ ),
    archive( archive_ ),
    status ( New ),
    hash( 0 )
{
    assert( archive );
    const QList<PersonPtr> authorList = article->authors();

    QString author;

    const PersonPtr firstAuthor = !authorList.isEmpty() ? authorList.first() : PersonPtr();

    hash = Utils::calcHash(article->title() + article->description() + article->content() + article->link() + author);

    guid = article->id();

    if (!archive->contains(guid))
    {
        archive->addEntry(guid);

        archive->setHash(guid, hash);
        QString title = article->title();
        if (title.isEmpty())
            title = buildTitle(article->description());
        archive->setTitle(guid, title);
        archive->setContent(guid, article->content());
        archive->setDescription(guid, article->description());
        archive->setLink(guid, article->link());
        //archive->setComments(guid, article.comments());
        //archive->setCommentsLink(guid, article.commentsLink().url());
        archive->setGuidIsPermaLink(guid, false);
        archive->setGuidIsHash(guid, guid.startsWith(QLatin1String("hash:")));
        const time_t datePublished = article->datePublished();
        if ( datePublished > 0 )
            pubDate.setTime_t( datePublished );
        else
            pubDate = QDateTime::currentDateTime();
        archive->setPubDate(guid, pubDate.toTime_t());
        if ( firstAuthor ) {
            archive->setAuthorName(guid, firstAuthor->name() );
            archive->setAuthorUri(guid, firstAuthor->uri() );
            archive->setAuthorEMail(guid, firstAuthor->email() );
        }
        const QList<EnclosurePtr> encs = article->enclosures();
        if ( !encs.isEmpty() )
        {
            archive->setEnclosure(guid, encs[0]->url(), encs[0]->type(), encs[0]->length());
        }
    }
    else
    {
        // always update comments count, as it's not used for hash calculation
        //archive->setComments(guid, article.comments());
        if (hash != archive->hash(guid)) //article is in archive, was it modified?
        { // if yes, update
            pubDate.setTime_t(archive->pubDate(guid));
            archive->setHash(guid, hash);
            QString title = article->title();
            if (title.isEmpty())
                title = buildTitle(article->description());
            archive->setTitle(guid, title);
            archive->setDescription(guid, article->description());
            archive->setContent(guid, article->content());
            archive->setLink(guid, article->link());
            if ( firstAuthor ) {
                archive->setAuthorName(guid, firstAuthor->name() );
                archive->setAuthorUri(guid, firstAuthor->uri() );
                archive->setAuthorEMail(guid, firstAuthor->email() );
            }
            //archive->setCommentsLink(guid, article.commentsLink());
        }
    }

    const QList<EnclosurePtr> encs = article->enclosures();
    if (!encs.isEmpty())
    {
        archive->setEnclosure(guid, encs[0]->url(), encs[0]->type(), encs[0]->length());
    }

}


Article::Article() : d( new Private )
{
}

Article::Article( const QString& guid, Feed* feed ) : d( new Private( guid, feed, feed->storage()->archiveFor( feed->xmlUrl() ) ) )
{
}

Article::Article( const ItemPtr& article, Feed* feed ) : d( new Private( article, feed, feed->storage()->archiveFor( feed->xmlUrl() ) ) )
{
}

Article::Article( const ItemPtr& article, Backend::FeedStorage* archive ) : d( new Private( article, 0, archive ) )
{
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

Article::Article(const Article &other) : d( other.d )
{
    d->ref();
}

Article::~Article()
{
    if ( d->deref() )
    {
        delete d;
        d = 0;
    }
}

Article &Article::operator=(const Article &other)
{
    Article copy( other );
    swap( copy );
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

bool Article::operator!=(const Article &other) const
{
    return d->guid != other.guid();
}

int Article::status() const
{
    if ((d->status & Private::Read) != 0)
        return Read;

    if ((d->status & Private::New) != 0)
        return New;

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
#ifdef CRASH_CHECKS
    if (!d->archive) KMessageBox::error(NULL, QString("Article::setStatus - article with GUID '%1' archive is NULL!").arg(d->guid));
    else
#endif // CRASH_CHECKS
        d->archive->setStatus(d->guid, d->status);
        if (d->feed)
            d->feed->setArticleChanged(*this, oldStatus);
     }
}

QString Article::title() const
{
#ifdef CRASH_CHECKS
    if (!d->archive)
    {
//        KMessageBox::error(NULL, QString("Article::title - article with GUID '%1' archive is NULL!").arg(d->guid));
        return ("(null)");
    }
#endif // CRASH_CHECKS
    return d->archive->title(d->guid);
}

QString Article::authorName() const
{
    return d->archive->authorName(d->guid);
}

QString Article::authorEMail() const
{
    return d->archive->authorEMail(d->guid);
}

QString Article::authorUri() const
{
    return d->archive->authorUri(d->guid);
}

QString Article::authorShort() const {
    const QString name = authorName();
    if ( !name.isEmpty() )
        return name;
    const QString email = authorEMail();
    if ( !email.isEmpty() )
        return email;
    const QString uri = authorUri();
    if ( !uri.isEmpty() )
        return uri;
    return QString();
}

QString Article::authorAsHtml() const {
    const QString name = authorName();
    const QString email = authorEMail();

    if (!email.isEmpty()) {
        if (!name.isEmpty())
            return QString("<a href=\"mailto:%1\">%2</a>").arg( email, name );
        else
            return QString("<a href=\"mailto:%1\">%1</a>").arg( email );
    }

    const QString uri = authorUri();
    if (!name.isEmpty()) {
        if (!uri.isEmpty())
            return QString("<a href=\"%1\">%2</a>").arg( uri, name );
        else
            return name;
    }

    if ( !uri.isEmpty() )
        return QString( "<a href=\"%1\">%1</a>" ).arg( uri );
    return QString();
}

KUrl Article::link() const
{
    return d->archive->link(d->guid);
}

QString Article::description() const
{
    return d->archive->description(d->guid);
}

QString Article::content( ContentOption opt ) const
{
    const QString cnt = d->archive->content( d->guid );
    return opt == ContentAndOnlyContent ? cnt : ( !cnt.isEmpty() ? cnt : description() );
}

QString Article::guid() const
{
    return d->guid;
}

KUrl Article::commentsLink() const
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

void Article::setKeep(bool keep)
{
    d->status = keep ? (d->status | Private::Keep) : (d->status & ~Private::Keep);
    d->archive->setStatus(d->guid, d->status);
    if (d->feed)
        d->feed->setArticleChanged(*this);
}

Feed* Article::feed() const
{ return d->feed; }

QDateTime Article::pubDate() const
{
    return d->pubDate;
}

shared_ptr<const Enclosure> Article::enclosure() const
{
    if ( !d->enclosure )
    {
        QString url;
        QString type;
        int length;
        bool hasEnc;
        d->archive->enclosure( d->guid, hasEnc, url, type, length );
        if ( hasEnc )
            d->enclosure.reset( new EnclosureImpl( url, type, static_cast<uint>( length ) ) );
        else
            d->enclosure.reset( new EnclosureImpl( QString(), QString(), 0 ) );
    }
    return d->enclosure;
}

} // namespace Akregator
