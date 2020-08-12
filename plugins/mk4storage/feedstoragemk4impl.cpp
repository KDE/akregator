/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "feedstoragemk4impl.h"
#include "storagemk4impl.h"

#include <Syndication/DocumentSource>
#include <Syndication/Global>
#include <Syndication/Feed>
#include <Syndication/Item>

#include <mk4.h>

#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>

namespace {
static uint calcHash(const QString &str)
{
    if (str.isNull()) { // handle null string as "", prevents crash
        return calcHash(QLatin1String(""));
    }
    const char *s = str.toLatin1().constData();
    uint hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;    // hash*33 + c
    }
    return hash;
}
}

namespace Akregator {
namespace Backend {
class FeedStorageMK4Impl::FeedStorageMK4ImplPrivate
{
public:
    FeedStorageMK4ImplPrivate()
        : modified(false)
        , pguid("guid")
        , ptitle("title")
        , pdescription("description")
        , pcontent("content")
        , plink("link")
        , pcommentsLink("commentsLink")
        , ptag("tag")
        , pEnclosureType("enclosureType")
        , pEnclosureUrl("enclosureUrl")
        , pcatTerm("catTerm")
        , pcatScheme("catScheme")
        , pcatName("catName")
        , pauthorName("authorName")
        , pauthorUri("authorUri")
        , pauthorEMail("authorEMail")
        , phash("hash")
        , pguidIsHash("guidIsHash")
        , pguidIsPermaLink("guidIsPermaLink")
        , pcomments("comments")
        , pstatus("status")
        , ppubDate("pubDate")
        , pHasEnclosure("hasEnclosure")
        , pEnclosureLength("enclosureLength")
    {
    }

    QString url;
    c4_Storage *storage;
    StorageMK4Impl *mainStorage;
    c4_View archiveView;

    bool autoCommit;
    bool modified;
    c4_StringProp pguid, ptitle, pdescription, pcontent, plink, pcommentsLink, ptag, pEnclosureType, pEnclosureUrl, pcatTerm, pcatScheme, pcatName, pauthorName, pauthorUri, pauthorEMail;
    c4_IntProp phash, pguidIsHash, pguidIsPermaLink, pcomments, pstatus, ppubDate, pHasEnclosure, pEnclosureLength;
};

FeedStorageMK4Impl::FeedStorageMK4Impl(const QString &url, StorageMK4Impl *main)
{
    d = new FeedStorageMK4ImplPrivate;
    d->autoCommit = main->autoCommit();
    d->url = url;
    d->mainStorage = main;

    QString url2 = url;

    if (url.length() > 255) {
        url2 = url.left(200) + QString::number(::calcHash(url), 16);
    }

    qDebug() << url2;
    QString t = url2;
    QString t2 = url2;
    QString filePath = main->archivePath() + QLatin1Char('/') + t.replace(QLatin1Char('/'), QLatin1Char('_')).replace(QLatin1Char(':'), QLatin1Char('_'));
    d->storage = new c4_Storage(QString(filePath + QLatin1String(".mk4")).toLocal8Bit().constData(), true);

    d->archiveView = d->storage->GetAs(
        "articles[guid:S,title:S,hash:I,guidIsHash:I,guidIsPermaLink:I,description:S,link:S,comments:I,commentsLink:S,status:I,pubDate:I,tags[tag:S],hasEnclosure:I,enclosureUrl:S,enclosureType:S,enclosureLength:I,categories[catTerm:S,catScheme:S,catName:S],authorName:S,content:S,authorUri:S,authorEMail:S]");

    c4_View hash = d->storage->GetAs("archiveHash[_H:I,_R:I]");
    d->archiveView = d->archiveView.Hash(hash, 1); // hash on guid
}

FeedStorageMK4Impl::~FeedStorageMK4Impl()
{
    delete d->storage;
    delete d;
    d = nullptr;
}

void FeedStorageMK4Impl::markDirty()
{
    if (!d->modified) {
        d->modified = true;
        // Tell this to mainStorage
        d->mainStorage->markDirty();
    }
}

void FeedStorageMK4Impl::commit()
{
    if (d->modified) {
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
    if (d->autoCommit) {
        commit();
    }
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

QDateTime FeedStorageMK4Impl::lastFetch() const
{
    return d->mainStorage->lastFetchFor(d->url);
}

void FeedStorageMK4Impl::setLastFetch(const QDateTime &lastFetch)
{
    d->mainStorage->setLastFetchFor(d->url, lastFetch);
}

QStringList FeedStorageMK4Impl::articles() const
{
    QStringList list;
    int size = d->archiveView.GetSize();
    list.reserve(size);
    for (int i = 0; i < size; ++i) {     // fill with guids
        list += QString::fromLatin1(d->pguid(d->archiveView.GetAt(i)));
    }
    return list;
}

void FeedStorageMK4Impl::addEntry(const QString &guid)
{
    c4_Row row;
    d->pguid(row) = guid.toLatin1().constData();
    if (!contains(guid)) {
        d->archiveView.Add(row);
        markDirty();
        setTotalCount(totalCount() + 1);
    }
}

bool FeedStorageMK4Impl::contains(const QString &guid) const
{
    return findArticle(guid) != -1;
}

int FeedStorageMK4Impl::findArticle(const QString &guid) const
{
    c4_Row findrow;
    d->pguid(findrow) = guid.toLatin1().constData();
    return d->archiveView.Find(findrow);
}

void FeedStorageMK4Impl::deleteArticle(const QString &guid)
{
    int findidx = findArticle(guid);
    if (findidx != -1) {
        setTotalCount(totalCount() - 1);
        d->archiveView.RemoveAt(findidx);
        markDirty();
    }
}

bool FeedStorageMK4Impl::guidIsHash(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsHash(d->archiveView.GetAt(findidx)) : false;
}

bool FeedStorageMK4Impl::guidIsPermaLink(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsPermaLink(d->archiveView.GetAt(findidx)) : false;
}

uint FeedStorageMK4Impl::hash(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->phash(d->archiveView.GetAt(findidx)) : 0;
}

void FeedStorageMK4Impl::setDeleted(const QString &guid)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }

    c4_Row row;
    row = d->archiveView.GetAt(findidx);
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

QString FeedStorageMK4Impl::link(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromLatin1(d->plink(d->archiveView.GetAt(findidx))) : QLatin1String("");
}

QDateTime FeedStorageMK4Impl::pubDate(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QDateTime::fromSecsSinceEpoch(d->ppubDate(d->archiveView.GetAt(findidx))) : QDateTime();
}

int FeedStorageMK4Impl::status(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pstatus(d->archiveView.GetAt(findidx)) : 0;
}

void FeedStorageMK4Impl::setStatus(const QString &guid, int status)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pstatus(row) = status;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::article(const QString &guid, uint &hash, QString &title, int &status, QDateTime &pubDate) const
{
    int idx = findArticle(guid);
    if (idx != -1) {
        auto view = d->archiveView.GetAt(idx);
        hash = d->phash(view);
        title = QString::fromUtf8(d->ptitle(view));
        status = d->pstatus(view);
        pubDate = QDateTime::fromSecsSinceEpoch(d->ppubDate(view));
    }
}

QString FeedStorageMK4Impl::title(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->ptitle(d->archiveView.GetAt(findidx))) : QLatin1String("");
}

QString FeedStorageMK4Impl::description(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pdescription(d->archiveView.GetAt(findidx))) : QLatin1String("");
}

QString FeedStorageMK4Impl::content(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pcontent(d->archiveView.GetAt(findidx))) : QLatin1String("");
}

void FeedStorageMK4Impl::setPubDate(const QString &guid, const QDateTime &pubdate)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->ppubDate(row) = pubdate.toSecsSinceEpoch();
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setGuidIsHash(const QString &guid, bool isHash)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pguidIsHash(row) = isHash;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setLink(const QString &guid, const QString &link)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->plink(row) = !link.isEmpty() ? link.toLatin1().constData() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setHash(const QString &guid, uint hash)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->phash(row) = hash;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setTitle(const QString &guid, const QString &title)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->ptitle(row) = !title.isEmpty() ? title.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setDescription(const QString &guid, const QString &description)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pdescription(row) = !description.isEmpty() ? description.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setContent(const QString &guid, const QString &content)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pcontent(row) = !content.isEmpty() ? content.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setAuthorName(const QString &guid, const QString &author)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pauthorName(row) = !author.isEmpty() ? author.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setAuthorUri(const QString &guid, const QString &author)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pauthorUri(row) = !author.isEmpty() ? author.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setAuthorEMail(const QString &guid, const QString &author)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pauthorEMail(row) = !author.isEmpty() ? author.toUtf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

QString FeedStorageMK4Impl::authorName(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pauthorName(d->archiveView.GetAt(findidx))) : QString();
}

QString FeedStorageMK4Impl::authorUri(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pauthorUri(d->archiveView.GetAt(findidx))) : QString();
}

QString FeedStorageMK4Impl::authorEMail(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pauthorEMail(d->archiveView.GetAt(findidx))) : QString();
}

void FeedStorageMK4Impl::setGuidIsPermaLink(const QString &guid, bool isPermaLink)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pguidIsPermaLink(row) = isPermaLink;
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::setEnclosure(const QString &guid, const QString &url, const QString &type, int length)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pHasEnclosure(row) = true;
    d->pEnclosureUrl(row) = !url.isEmpty() ? url.toUtf8().data() : "";
    d->pEnclosureType(row) = !type.isEmpty() ? type.toUtf8().data() : "";
    d->pEnclosureLength(row) = length;

    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::removeEnclosure(const QString &guid)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pHasEnclosure(row) = false;
    d->pEnclosureUrl(row) = "";
    d->pEnclosureType(row) = "";
    d->pEnclosureLength(row) = -1;

    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorageMK4Impl::enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length) const
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
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

void FeedStorageMK4Impl::setCategories(const QString &, const QStringList &categories)
{
    //TODO
}

QStringList FeedStorageMK4Impl::categories(const QString &guid) const
{
    //TODO
    return {};
}
} // namespace Backend
} // namespace Akregator
