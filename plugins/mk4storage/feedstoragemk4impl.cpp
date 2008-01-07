/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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

#include "feedstoragemk4impl.h"
#include "storagemk4impl.h"

#include <syndication/documentsource.h>
#include <syndication/global.h>
#include <syndication/feed.h>
#include <syndication/item.h>

#include <mk4.h>

#include <qdom.h>
#include <QFile>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

namespace {
static uint calcHash(const QString& str)
{
    if (str.isNull()) // handle null string as "", prevents crash
        return calcHash("");
    const char* s = str.toAscii();
    uint hash = 5381;
    int c;
    while ( ( c = *s++ ) ) hash = ((hash << 5) + hash) + c; // hash*33 + c
    return hash;
}
}


namespace Akregator {
namespace Backend {

class FeedStorageMK4Impl::FeedStorageMK4ImplPrivate
{
    public:
        FeedStorageMK4ImplPrivate() :
            modified(false),
            pguid("guid"),
            ptitle("title"),
            pdescription("description"),
            plink("link"),
            pcommentsLink("commentsLink"),
            ptag("tag"),
            pEnclosureType("enclosureType"),
            pEnclosureUrl("enclosureUrl"),
            pcatTerm("catTerm"),
            pcatScheme("catScheme"),
            pcatName("catName"),
            pauthor("author"),
            phash("hash"),
            pguidIsHash("guidIsHash"),
            pguidIsPermaLink("guidIsPermaLink"),
            pcomments("comments"),
            pstatus("status"),
            ppubDate("pubDate"),
            pHasEnclosure("hasEnclosure"),
            pEnclosureLength("enclosureLength"),
            ptags("tags"),
            ptaggedArticles("taggedArticles"),
            pcategorizedArticles("categorizedArticles"),
            pcategories("categories")
        {}

        QString url;
        c4_Storage* storage;
        StorageMK4Impl* mainStorage;
        c4_View archiveView;

        c4_Storage* catStorage;
        c4_View catView;
        c4_Storage* tagStorage;
        c4_View tagView;
        bool autoCommit;
	    bool modified;
        bool convert;
        QString oldArchivePath;
        c4_StringProp pguid, ptitle, pdescription, plink, pcommentsLink, ptag, pEnclosureType, pEnclosureUrl, pcatTerm, pcatScheme, pcatName, pauthor;
        c4_IntProp phash, pguidIsHash, pguidIsPermaLink, pcomments, pstatus, ppubDate, pHasEnclosure, pEnclosureLength;
        c4_ViewProp ptags, ptaggedArticles, pcategorizedArticles, pcategories;
};

void FeedStorageMK4Impl::convertOldArchive()
{
    if (!d->convert)
        return;

    d->convert = false;
    QFile file(d->oldArchivePath);

    if ( !file.open(QIODevice::ReadOnly) )
        return;

    Syndication::DocumentSource src(file.readAll(), "http://foo");
    file.close();
    Syndication::FeedPtr feed = Syndication::parse(src);

    if (feed)
    {
        QList<Syndication::ItemPtr> items = feed->items();
        QList<Syndication::ItemPtr>::ConstIterator it = items.begin();
        QList<Syndication::ItemPtr>::ConstIterator en = items.end();
        d->modified = true;
        commit();
    }
}

FeedStorageMK4Impl::FeedStorageMK4Impl(const QString& url, StorageMK4Impl* main)
{
    d = new FeedStorageMK4ImplPrivate;
    d->autoCommit = main->autoCommit();
    d->url = url;
    d->mainStorage = main;

    QString url2 = url;

    if (url.length() > 255)
    {
        url2 = url.left(200) + QString::number(::calcHash(url), 16);
    }

    kDebug() << url2;
    QString t = url2;
    QString t2 = url2;
    QString filePath = main->archivePath() + '/' + t.replace('/', '_').replace(':', '_');
    d->oldArchivePath = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + t2.replace('/', '_').replace(':', '_') + ".xml";
    d->convert = !QFile::exists(filePath + ".mk4") && QFile::exists(d->oldArchivePath);
    d->storage = new c4_Storage((filePath + ".mk4").toLocal8Bit(), true);

    d->archiveView = d->storage->GetAs("articles[guid:S,title:S,hash:I,guidIsHash:I,guidIsPermaLink:I,description:S,link:S,comments:I,commentsLink:S,status:I,pubDate:I,tags[tag:S],hasEnclosure:I,enclosureUrl:S,enclosureType:S,enclosureLength:I,categories[catTerm:S,catScheme:S,catName:S],author:S]");

    c4_View hash = d->storage->GetAs("archiveHash[_H:I,_R:I]");
    d->archiveView = d->archiveView.Hash(hash, 1); // hash on guid


    d->tagStorage = new c4_Storage((filePath + ".mk4___TAGS").toLocal8Bit(), true);
    d->tagView = d->tagStorage->GetAs("tagIndex[tag:S,taggedArticles[guid:S]]");
    hash = d->tagStorage->GetAs("archiveHash[_H:I,_R:I]");
    d->tagView = d->tagView.Hash(hash, 1); // hash on tag

    d->catStorage = new c4_Storage((filePath + ".mk4___CATEGORIES").toLocal8Bit(), true);
    d->catView = d->catStorage->GetAs("catIndex[catTerm:S,catScheme:S,catName:S,categorizedArticles[guid:S]]");
}


FeedStorageMK4Impl::~FeedStorageMK4Impl()
{
    delete d->storage;
    delete d->tagStorage;
    delete d->catStorage;
    delete d; d = 0;
}

void FeedStorageMK4Impl::commit()
{
    if (d->modified)
    {
        d->storage->Commit();
        d->tagStorage->Commit();
        d->catStorage->Commit();
    }
    d->modified = false;
}

void FeedStorageMK4Impl::rollback()
{
    d->storage->Rollback();
    d->tagStorage->Rollback();
    d->catStorage->Rollback();
}

void FeedStorageMK4Impl::close()
{
    if (d->autoCommit)
        commit();
}
int FeedStorageMK4Impl::unread() const
{
    return d->mainStorage->unreadFor(d->url);
}
void FeedStorageMK4Impl::setUnread(int unread)
{
    d->mainStorage->setUnreadFor(d->url, unread);
}

int FeedStorageMK4Impl::totalCount() const
{
    return d->mainStorage->totalCountFor(d->url);
}

void FeedStorageMK4Impl::setTotalCount(int total)
{
    d->mainStorage->setTotalCountFor(d->url, total);
}

int FeedStorageMK4Impl::lastFetch() const
{
    return d->mainStorage->lastFetchFor(d->url);
}

void FeedStorageMK4Impl::setLastFetch(int lastFetch)
{
    d->mainStorage->setLastFetchFor(d->url, lastFetch);
}

QStringList FeedStorageMK4Impl::articles(const QString& tag) const
{
    QStringList list;
    if (tag.isNull()) // return all articles
    {
        int size = d->archiveView.GetSize();
        for (int i = 0; i < size; i++) // fill with guids
            list += QString(d->pguid(d->archiveView.GetAt(i)));
    }
    else
    {
        c4_Row tagrow;
        d->ptag(tagrow) = tag.toUtf8().data();
        int tagidx = d->tagView.Find(tagrow);
        if (tagidx != -1)
        {
            tagrow = d->tagView.GetAt(tagidx);
            c4_View tagView = d->ptaggedArticles(tagrow);
            int size = tagView.GetSize();
            for (int i = 0; i < size; i++)
                list += QString(d->pguid(tagView.GetAt(i)));
        }

    }
    return list;
}

QStringList FeedStorageMK4Impl::articles(const Category& cat) const
{
    QStringList list;

    c4_Row catrow;
    d->pcatTerm(catrow) = cat.term.toUtf8().data();
    d->pcatScheme(catrow) = cat.scheme.toUtf8().data();

    int catidx = d->catView.Find(catrow);
    if (catidx != -1)
    {
        catrow = d->catView.GetAt(catidx);
        c4_View catView = d->pcategorizedArticles(catrow);
        int size = catView.GetSize();
        for (int i = 0; i < size; i++)
            list += QString(d->pguid(catView.GetAt(i)));
    }

    return list;
}

void FeedStorageMK4Impl::addEntry(const QString& guid)
{
    c4_Row row;
    d->pguid(row) = guid.toAscii();
    if (!contains(guid))
    {
        d->archiveView.Add(row);
	d->modified = true;
        setTotalCount(totalCount()+1);
    }
}

bool FeedStorageMK4Impl::contains(const QString& guid) const
{
    return findArticle(guid) != -1;
}

int FeedStorageMK4Impl::findArticle(const QString& guid) const
{
    c4_Row findrow;
    d->pguid(findrow) = guid.toAscii();
    return d->archiveView.Find(findrow);
}

void FeedStorageMK4Impl::deleteArticle(const QString& guid)
{

    int findidx = findArticle(guid);
    if (findidx != -1)
    {
        QStringList list = tags(guid);
        for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
            removeTag(guid, *it);
        setTotalCount(totalCount()-1);
        d->archiveView.RemoveAt(findidx);
        d->modified = true;
    }
}

int FeedStorageMK4Impl::comments(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pcomments(d->archiveView.GetAt(findidx)) : 0;
}

QString FeedStorageMK4Impl::commentsLink(const QString& guid) const
{
   int findidx = findArticle(guid);
   return findidx != -1 ? QString(d->pcommentsLink(d->archiveView.GetAt(findidx))) : "";
}

bool FeedStorageMK4Impl::guidIsHash(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsHash(d->archiveView.GetAt(findidx)) : false;
}

bool FeedStorageMK4Impl::guidIsPermaLink(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsPermaLink(d->archiveView.GetAt(findidx)) : false;
}

uint FeedStorageMK4Impl::hash(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->phash(d->archiveView.GetAt(findidx)) : 0;
}


void FeedStorageMK4Impl::setDeleted(const QString& guid)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;

    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    QStringList list = tags(guid);
        for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
            removeTag(guid, *it);
    d->pdescription(row) = "";
    d->ptitle(row) = "";
    d->plink(row) = "";
    d->pauthor(row) = "";
    d->pcommentsLink(row) = "";
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

QString FeedStorageMK4Impl::link(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString(d->plink(d->archiveView.GetAt(findidx))) : "";
}

uint FeedStorageMK4Impl::pubDate(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->ppubDate(d->archiveView.GetAt(findidx)) : 0;
}

int FeedStorageMK4Impl::status(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pstatus(d->archiveView.GetAt(findidx)) : 0;
}

void FeedStorageMK4Impl::setStatus(const QString& guid, int status)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pstatus(row) = status;
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

QString FeedStorageMK4Impl::title(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->ptitle(d->archiveView.GetAt(findidx))) : "";
}

QString FeedStorageMK4Impl::description(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pdescription(d->archiveView.GetAt(findidx))) : "";
}


void FeedStorageMK4Impl::setPubDate(const QString& guid, uint pubdate)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->ppubDate(row) = pubdate;
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::setGuidIsHash(const QString& guid, bool isHash)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pguidIsHash(row) = isHash;
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::setLink(const QString& guid, const QString& link)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->plink(row) = !link.isEmpty() ? link.toAscii() : "";
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::setHash(const QString& guid, uint hash)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->phash(row) = hash;
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::setTitle(const QString& guid, const QString& title)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->ptitle(row) = !title.isEmpty() ? title.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::setDescription(const QString& guid, const QString& description)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pdescription(row) = !description.isEmpty() ? description.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::setAuthor(const QString& guid, const QString& author)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pauthor(row) = !author.isEmpty() ? author.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

QString FeedStorageMK4Impl::author(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pauthor(d->archiveView.GetAt(findidx))) : "";
}


void FeedStorageMK4Impl::setCommentsLink(const QString& guid, const QString& commentsLink)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pcommentsLink(row) = !commentsLink.isEmpty() ? commentsLink.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::setComments(const QString& guid, int comments)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pcomments(row) = comments;
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}


void FeedStorageMK4Impl::setGuidIsPermaLink(const QString& guid, bool isPermaLink)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pguidIsPermaLink(row) = isPermaLink;
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::addCategory(const QString& guid, const Category& cat)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;

    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    c4_View catView = d->pcategories(row);
    c4_Row findrow;

    d->pcatTerm(findrow) = cat.term.toUtf8().data();
    d->pcatScheme(findrow) = cat.scheme.toUtf8().data();

    int catidx = catView.Find(findrow);
    if (catidx == -1)
    {
        d->pcatName(findrow) = cat.name.toUtf8().data();
        catidx = catView.Add(findrow);
        d->pcategories(row) = catView;
        d->archiveView.SetAt(findidx, row);

        // add to category->articles index
        c4_Row catrow;
        d->pcatTerm(catrow) = cat.term.toUtf8().data();
        d->pcatScheme(catrow) = cat.scheme.toUtf8().data();
        d->pcatName(catrow) = cat.name.toUtf8().data();

        int catidx2 = d->catView.Find(catrow);

        if (catidx2 == -1)
        {
            catidx2 = d->catView.Add(catrow);
        }

        c4_Row catrow2 = d->catView.GetAt(catidx2);
        c4_View catView2 = d->pcategorizedArticles(catrow2);

        c4_Row row3;
        d->pguid(row3) = guid.toAscii();
        int guididx = catView2.Find(row3);
        if (guididx == -1)
        {
            guididx = catView2.Add(row3);
            catView2.SetAt(guididx, row3);
            d->pcategorizedArticles(catrow2) = catView2;
            d->catView.SetAt(catidx2, catrow2);
        }

        d->modified = true;
    }
}

QList<Category> FeedStorageMK4Impl::categories(const QString& guid) const
{

    QList<Category> list;

    if (!guid.isNull()) // return categories for an article
    {
        int findidx = findArticle(guid);
        if (findidx == -1)
            return list;

        c4_Row row;
        row = d->archiveView.GetAt(findidx);
        c4_View catView = d->pcategories(row);
        int size = catView.GetSize();

        for (int i = 0; i < size; ++i)
        {
            Category cat;

            cat.term = QString::fromUtf8(d->pcatTerm(catView.GetAt(i)));
            cat.scheme = QString::fromUtf8(d->pcatScheme(catView.GetAt(i)));
            cat.name = QString::fromUtf8(d->pcatName(catView.GetAt(i)));

            list += cat;
        }
    }
    else // return all categories in the feed
    {
        int size = d->catView.GetSize();
        for (int i = 0; i < size; i++)
        {
            c4_Row row = d->catView.GetAt(i);

            Category cat;
            cat.term = QString(d->pcatTerm(row));
            cat.scheme = QString(d->pcatScheme(row));
            cat.name = QString(d->pcatName(row));

            list += cat;
        }
    }

    return list;
}

void FeedStorageMK4Impl::addTag(const QString& guid, const QString& tag)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;

    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    c4_View tagView = d->ptags(row);
    c4_Row findrow;
    d->ptag(findrow) = tag.toUtf8().data();
    int tagidx = tagView.Find(findrow);
    if (tagidx == -1)
    {
        tagidx = tagView.Add(findrow);
        d->ptags(row) = tagView;
        d->archiveView.SetAt(findidx, row);

        // add to tag->articles index
        c4_Row tagrow;
        d->ptag(tagrow) = tag.toUtf8().data();
        int tagidx2 = d->tagView.Find(tagrow);
        if (tagidx2 == -1)
            tagidx2 = d->tagView.Add(tagrow);
        tagrow = d->tagView.GetAt(tagidx2);
        c4_View tagView2 = d->ptaggedArticles(tagrow);

        c4_Row row3;
        d->pguid(row3) = guid.toAscii();
        int guididx = tagView2.Find(row3);
        if (guididx == -1)
        {
            guididx = tagView2.Add(row3);
            tagView2.SetAt(guididx, row3);
            d->ptaggedArticles(tagrow) = tagView2;
            d->tagView.SetAt(tagidx2, tagrow);
        }
        d->modified = true;
    }
}

void FeedStorageMK4Impl::removeTag(const QString& guid, const QString& tag)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;

    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    c4_View tagView = d->ptags(row);
    c4_Row findrow;
    d->ptag(findrow) = tag.toUtf8().data();
    int tagidx = tagView.Find(findrow);
    if (tagidx != -1)
    {
        tagView.RemoveAt(tagidx);
        d->ptags(row) = tagView;
        d->archiveView.SetAt(findidx, row);

        // remove from tag->articles index
        c4_Row tagrow;
        d->ptag(tagrow) = tag.toUtf8().data();
        int tagidx2 = d->tagView.Find(tagrow);
        if (tagidx2 != -1)
        {
            tagrow = d->tagView.GetAt(tagidx2);
            c4_View tagView2 = d->ptaggedArticles(tagrow);

            c4_Row row3;
            d->pguid(row3) = guid.toAscii();
            int guididx = tagView2.Find(row3);
            if (guididx != -1)
            {
                tagView2.RemoveAt(guididx);
                d->ptaggedArticles(tagrow) = tagView2;
                d->tagView.SetAt(tagidx2, tagrow);
            }
        }

        d->modified = true;
    }
}

QStringList FeedStorageMK4Impl::tags(const QString& guid) const
{
    QStringList list;

    if (!guid.isNull()) // return tags for an articles
    {
        int findidx = findArticle(guid);
        if (findidx == -1)
            return list;

        c4_Row row;
        row = d->archiveView.GetAt(findidx);
        c4_View tagView = d->ptags(row);
        int size = tagView.GetSize();

        for (int i = 0; i < size; ++i)
            list += QString::fromUtf8(d->ptag(tagView.GetAt(i)));
    }
    else // return all tags in the feed
    {
        int size = d->tagView.GetSize();
        for (int i = 0; i < size; i++)
             list += QString(d->ptag(d->tagView.GetAt(i)));
    }

    return list;
}

void FeedStorageMK4Impl::add(FeedStorage* source)
{
    QStringList articles = source->articles();
    for (QStringList::ConstIterator it = articles.begin(); it != articles.end(); ++it)
        copyArticle(*it, source);
    setUnread(source->unread());
    setLastFetch(source->lastFetch());
    setTotalCount(source->totalCount());
}

void FeedStorageMK4Impl::copyArticle(const QString& guid, FeedStorage* source)
{
    if (!contains(guid))
        addEntry(guid);
    setComments(guid, source->comments(guid));
    setCommentsLink(guid, source->commentsLink(guid));
    setDescription(guid, source->description(guid));
    setGuidIsHash(guid, source->guidIsHash(guid));
    setGuidIsPermaLink(guid, source->guidIsPermaLink(guid));
    setHash(guid, source->hash(guid));
    setLink(guid, source->link(guid));
    setPubDate(guid, source->pubDate(guid));
    setStatus(guid, source->status(guid));
    setTitle(guid, source->title(guid));
    setAuthor(guid, source->author(guid));

    QStringList tags = source->tags(guid);
    for (QStringList::ConstIterator it = tags.begin(); it != tags.end(); ++it)
        addTag(guid, *it);
}

void FeedStorageMK4Impl::setEnclosure(const QString& guid, const QString& url, const QString& type, int length)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pHasEnclosure(row) = true;
    d->pEnclosureUrl(row) = !url.isEmpty() ? url.toUtf8().data() : "";
    d->pEnclosureType(row) = !type.isEmpty() ? type.toUtf8().data() : "";
    d->pEnclosureLength(row) = length;

    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::removeEnclosure(const QString& guid)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pHasEnclosure(row) = false;
    d->pEnclosureUrl(row) = "";
    d->pEnclosureType(row) = "";
    d->pEnclosureLength(row) = -1;

    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::enclosure(const QString& guid, bool& hasEnclosure, QString& url, QString& type, int& length) const
{
    int findidx = findArticle(guid);
    if (findidx == -1)
    {
        hasEnclosure = false;
        url.clear();
        type.clear();
        length = -1;
        return;
    }
    c4_Row row = d->archiveView.GetAt(findidx);
    hasEnclosure = d->pHasEnclosure(row);
    url = d->pEnclosureUrl(row);
    type = d->pEnclosureType(row);
    length = d->pEnclosureLength(row);
}

void FeedStorageMK4Impl::clear()
{
    d->storage->RemoveAll();
    d->tagStorage->RemoveAll();
    setUnread(0);
    d->modified = true;
}

} // namespace Backend
} // namespace Akregator
