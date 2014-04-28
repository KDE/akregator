/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

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

#include <Syndication/documentsource.h>
#include <Syndication/global.h>
#include <Syndication/feed.h>
#include <Syndication/item.h>

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
        return calcHash(QLatin1String(""));
    const char* s = str.toLatin1();
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
            pcontent("content"),
            plink("link"),
            pcommentsLink("commentsLink"),
            ptag("tag"),
            pEnclosureType("enclosureType"),
            pEnclosureUrl("enclosureUrl"),
            pcatTerm("catTerm"),
            pcatScheme("catScheme"),
            pcatName("catName"),
            pauthorName("authorName"),
            pauthorUri("authorUri"),
            pauthorEMail("authorEMail"),
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

        bool autoCommit;
        bool modified;
        bool convert;
        QString oldArchivePath;
        c4_StringProp pguid, ptitle, pdescription, pcontent, plink, pcommentsLink, ptag, pEnclosureType, pEnclosureUrl, pcatTerm, pcatScheme, pcatName, pauthorName, pauthorUri, pauthorEMail;
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

    Syndication::DocumentSource src(file.readAll(), QLatin1String("http://foo"));
    file.close();
    Syndication::FeedPtr feed = Syndication::parse(src);

    if (feed)
    {
        //QList<Syndication::ItemPtr> items = feed->items();
        //QList<Syndication::ItemPtr>::ConstIterator it = items.constBegin();
        //QList<Syndication::ItemPtr>::ConstIterator en = items.constEnd();
        markDirty();
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
    QString filePath = main->archivePath() + QLatin1Char('/') + t.replace(QLatin1Char('/'), QLatin1Char('_')).replace(QLatin1Char(':'), QLatin1Char('_'));
    d->oldArchivePath = KGlobal::dirs()->saveLocation("data", QLatin1String("akregator/Archive/")) + t2.replace(QLatin1Char('/'), QLatin1Char('_')).replace(QLatin1Char(':'), QLatin1Char('_')) + QLatin1String(".xml");
    d->convert = !QFile::exists(filePath + QLatin1String(".mk4")) && QFile::exists(d->oldArchivePath);
    d->storage = new c4_Storage(QString(filePath + QLatin1String(".mk4")).toLocal8Bit(), true);

    d->archiveView = d->storage->GetAs("articles[guid:S,title:S,hash:I,guidIsHash:I,guidIsPermaLink:I,description:S,link:S,comments:I,commentsLink:S,status:I,pubDate:I,tags[tag:S],hasEnclosure:I,enclosureUrl:S,enclosureType:S,enclosureLength:I,categories[catTerm:S,catScheme:S,catName:S],authorName:S,content:S,authorUri:S,authorEMail:S]");

    c4_View hash = d->storage->GetAs("archiveHash[_H:I,_R:I]");
    d->archiveView = d->archiveView.Hash(hash, 1); // hash on guid
}


FeedStorageMK4Impl::~FeedStorageMK4Impl()
{
    delete d->storage;
    delete d; d = 0;
}

void FeedStorageMK4Impl::markDirty()
{
    if (!d->modified)
    {
        d->modified = true;
        // Tell this to mainStorage
        d->mainStorage->markDirty();
    }
}

void FeedStorageMK4Impl::commit()
{
    if (d->modified)
    {
        d->storage->Commit();
    }
    d->modified = false;
}

void FeedStorageMK4Impl::rollback()
{
    d->storage->Rollback();
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
#if 0 //category and tag support disabled
    if (tag.isNull()) // return all articles
    {
#endif
        int size = d->archiveView.GetSize();
        for (int i = 0; i < size; ++i) // fill with guids
            list += QString::fromLatin1(d->pguid(d->archiveView.GetAt(i)));
#if 0 //category and tag support disabled
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
            for (int i = 0; i < size; ++i)
                list += QString(d->pguid(tagView.GetAt(i)));
        }

    }
#endif
    return list;
}

QStringList FeedStorageMK4Impl::articles(const Category& cat) const
{
    QStringList list;
#if 0 //category and tag support disabled
    c4_Row catrow;
    d->pcatTerm(catrow) = cat.term.toUtf8().data();
    d->pcatScheme(catrow) = cat.scheme.toUtf8().data();

    int catidx = d->catView.Find(catrow);
    if (catidx != -1)
    {
        catrow = d->catView.GetAt(catidx);
        c4_View catView = d->pcategorizedArticles(catrow);
        int size = catView.GetSize();
        for (int i = 0; i < size; ++i)
            list += QString(d->pguid(catView.GetAt(i)));
    }
#endif
    return list;
}

void FeedStorageMK4Impl::addEntry(const QString& guid)
{
    c4_Row row;
    d->pguid(row) = guid.toLatin1();
    if (!contains(guid))
    {
        d->archiveView.Add(row);
        markDirty();
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
    d->pguid(findrow) = guid.toLatin1();
    return d->archiveView.Find(findrow);
}

void FeedStorageMK4Impl::deleteArticle(const QString& guid)
{

    int findidx = findArticle(guid);
    if (findidx != -1)
    {
        QStringList list = tags(guid);
        for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
            removeTag(guid, *it);
        setTotalCount(totalCount()-1);
        d->archiveView.RemoveAt(findidx);
        markDirty();
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
   return findidx != -1 ? QString::fromLatin1(d->pcommentsLink(d->archiveView.GetAt(findidx))) : QLatin1String("");
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
        for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
            removeTag(guid, *it);
    d->pdescription(row) = "";
    d->pcontent(row) = "";
    d->ptitle(row) = "";
    d->plink(row) = "";
    d->pauthorName(row) = "";
    d->pauthorUri(row) = "";
    d->pauthorEMail(row) = "";
    d->pcommentsLink(row) = "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

QString FeedStorageMK4Impl::link(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromLatin1(d->plink(d->archiveView.GetAt(findidx))) : QLatin1String("");
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
    markDirty();
}

QString FeedStorageMK4Impl::title(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->ptitle(d->archiveView.GetAt(findidx))) : QLatin1String("");
}

QString FeedStorageMK4Impl::description(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pdescription(d->archiveView.GetAt(findidx))) : QLatin1String("");
}

QString FeedStorageMK4Impl::content(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pcontent(d->archiveView.GetAt(findidx))) : QLatin1String("");
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
    markDirty();
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
    markDirty();
}

void FeedStorageMK4Impl::setLink(const QString& guid, const QString& link)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->plink(row) = !link.isEmpty() ? link.toLatin1() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
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
    markDirty();
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
    markDirty();
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
    markDirty();
}

void FeedStorageMK4Impl::setContent(const QString& guid, const QString& content)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pcontent(row) = !content.isEmpty() ? content.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setAuthorName(const QString& guid, const QString& author)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pauthorName(row) = !author.isEmpty() ? author.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}


void FeedStorageMK4Impl::setAuthorUri(const QString& guid, const QString& author)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pauthorUri(row) = !author.isEmpty() ? author.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}


void FeedStorageMK4Impl::setAuthorEMail(const QString& guid, const QString& author)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pauthorEMail(row) = !author.isEmpty() ? author.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

QString FeedStorageMK4Impl::authorName(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pauthorName(d->archiveView.GetAt(findidx))) : QLatin1String("");
}

QString FeedStorageMK4Impl::authorUri(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pauthorUri(d->archiveView.GetAt(findidx))) : QLatin1String("");
}

QString FeedStorageMK4Impl::authorEMail(const QString& guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pauthorEMail(d->archiveView.GetAt(findidx))) : QLatin1String("");
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
    markDirty();
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
    markDirty();
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
    markDirty();
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
#if 0 //category and tag support disabled
        int catidx2 = d->catView.Find(catrow);

        if (catidx2 == -1)
        {
            catidx2 = d->catView.Add(catrow);
        }

        c4_Row catrow2 = d->catView.GetAt(catidx2);
        c4_View catView2 = d->pcategorizedArticles(catrow2);

        c4_Row row3;
        d->pguid(row3) = guid.toLatin1();
        int guididx = catView2.Find(row3);
        if (guididx == -1)
        {
            guididx = catView2.Add(row3);
            catView2.SetAt(guididx, row3);
            d->pcategorizedArticles(catrow2) = catView2;
            d->catView.SetAt(catidx2, catrow2);
        }
#endif
        markDirty();
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
#if 0 //category and tag support disabled
        int size = d->catView.GetSize();
        for (int i = 0; i < size; ++i)
        {
            c4_Row row = d->catView.GetAt(i);

            Category cat;
            cat.term = QString(d->pcatTerm(row));
            cat.scheme = QString(d->pcatScheme(row));
            cat.name = QString(d->pcatName(row));

            list += cat;
        }
#endif

    }

    return list;
}

void FeedStorageMK4Impl::addTag(const QString& guid, const QString& tag)
{
#if 0 //category and tag support disabled
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
        d->pguid(row3) = guid.toLatin1();
        int guididx = tagView2.Find(row3);
        if (guididx == -1)
        {
            guididx = tagView2.Add(row3);
            tagView2.SetAt(guididx, row3);
            d->ptaggedArticles(tagrow) = tagView2;
            d->tagView.SetAt(tagidx2, tagrow);
        }
        markDirty();
    }
#endif
}

void FeedStorageMK4Impl::removeTag(const QString& guid, const QString& tag)
{
#if 0 //category and tag support disabled
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
            d->pguid(row3) = guid.toLatin1();
            int guididx = tagView2.Find(row3);
            if (guididx != -1)
            {
                tagView2.RemoveAt(guididx);
                d->ptaggedArticles(tagrow) = tagView2;
                d->tagView.SetAt(tagidx2, tagrow);
            }
        }

        markDirty();
    }
#endif
}

QStringList FeedStorageMK4Impl::tags(const QString& guid) const
{
    Q_UNUSED( guid );
//TODO what is the need of the unused parameter(s)?
    QStringList list;
#if 0 //category and tag support disabled
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
        for (int i = 0; i < size; ++i)
             list += QString(d->ptag(d->tagView.GetAt(i)));
    }
#endif
    return list;
}

void FeedStorageMK4Impl::add(FeedStorage* source)
{
    QStringList articles = source->articles();
    for (QStringList::ConstIterator it = articles.constBegin(); it != articles.constEnd(); ++it)
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
    setAuthorName(guid, source->authorName(guid));
    setAuthorUri(guid, source->authorUri(guid));
    setAuthorEMail(guid, source->authorEMail(guid));

    QStringList tags = source->tags(guid);
    for (QStringList::ConstIterator it = tags.constBegin(); it != tags.constEnd(); ++it)
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
    markDirty();
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
    markDirty();
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
    url = QLatin1String(d->pEnclosureUrl(row));
    type = QLatin1String(d->pEnclosureType(row));
    length = d->pEnclosureLength(row);
}

void FeedStorageMK4Impl::clear()
{
    d->storage->RemoveAll();

    setUnread(0);
    markDirty();
}

} // namespace Backend
} // namespace Akregator

