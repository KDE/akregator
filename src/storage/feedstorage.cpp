/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "feedstorage.h"
#include "storage.h"

#include <Syndication/DocumentSource>
#include <Syndication/Feed>
#include <Syndication/Item>

#include "mk4.h"

#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>

namespace
{
static uint calcHash(const QString &str)
{
    if (str.isNull()) { // handle null string as "", prevents crash
        return calcHash(QLatin1StringView(""));
    }
    const char *s = str.toLatin1().constData();
    uint hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c; // hash*33 + c
    }
    return hash;
}
}

namespace Akregator
{
namespace Backend
{
class FeedStorage::FeedStoragePrivate
{
public:
    FeedStoragePrivate()
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
    Storage *mainStorage;
    c4_View archiveView;

    bool autoCommit;
    bool modified;
    c4_StringProp pguid, ptitle, pdescription, pcontent, plink, pcommentsLink, ptag, pEnclosureType, pEnclosureUrl, pcatTerm, pcatScheme, pcatName, pauthorName,
        pauthorUri, pauthorEMail;
    c4_IntProp phash, pguidIsHash, pguidIsPermaLink, pcomments, pstatus, ppubDate, pHasEnclosure, pEnclosureLength;
};

FeedStorage::FeedStorage(const QString &url, Storage *main)
    : d(new FeedStoragePrivate)
{
    d->autoCommit = main->autoCommit();
    d->url = url;
    d->mainStorage = main;

    QString url2 = url;

    if (url.length() > 255) {
        url2 = url.left(200) + QString::number(::calcHash(url), 16);
    }

    qDebug() << url2;
    QString t = url2;
    const QString filePath = main->archivePath() + QLatin1Char('/') + t.replace(QLatin1Char('/'), QLatin1Char('_')).replace(QLatin1Char(':'), u'_');
    d->storage = new c4_Storage(QString(filePath + QLatin1StringView(".mk4")).toLocal8Bit().constData(), static_cast<int>(true));

    d->archiveView = d->storage->GetAs(
        "articles[guid:S,title:S,hash:I,guidIsHash:I,guidIsPermaLink:I,description:S,link:S,comments:I,commentsLink:S,status:I,pubDate:I,tags[tag:S],"
        "hasEnclosure:I,enclosureUrl:S,enclosureType:S,enclosureLength:I,categories[catTerm:S,catScheme:S,catName:S],authorName:S,content:S,authorUri:S,"
        "authorEMail:S]");

    c4_View hash = d->storage->GetAs("archiveHash[_H:I,_R:I]");
    d->archiveView = d->archiveView.Hash(hash, 1); // hash on guid
}

FeedStorage::~FeedStorage()
{
    delete d->storage;
}

void FeedStorage::markDirty()
{
    if (!d->modified) {
        d->modified = true;
        // Tell this to mainStorage
        d->mainStorage->markDirty();
    }
}

void FeedStorage::commit()
{
    if (d->modified) {
        d->storage->Commit();
    }
    d->modified = false;
}

void FeedStorage::rollback()
{
    d->storage->Rollback();
}

void FeedStorage::close()
{
    if (d->autoCommit) {
        commit();
    }
}

int FeedStorage::unread() const
{
    return d->mainStorage->unreadFor(d->url);
}

void FeedStorage::setUnread(int unread)
{
    d->mainStorage->setUnreadFor(d->url, unread);
}

int FeedStorage::totalCount() const
{
    return d->mainStorage->totalCountFor(d->url);
}

void FeedStorage::setTotalCount(int total)
{
    d->mainStorage->setTotalCountFor(d->url, total);
}

QDateTime FeedStorage::lastFetch() const
{
    return d->mainStorage->lastFetchFor(d->url);
}

void FeedStorage::setLastFetch(const QDateTime &lastFetch)
{
    d->mainStorage->setLastFetchFor(d->url, lastFetch);
}

QStringList FeedStorage::articles() const
{
    QStringList list;
    int size = d->archiveView.GetSize();
    list.reserve(size);
    for (int i = 0; i < size; ++i) { // fill with guids
        list += QString::fromLatin1(QByteArray(d->pguid(d->archiveView.GetAt(i))));
    }
    return list;
}

void FeedStorage::addEntry(const QString &guid)
{
    c4_Row row;
    d->pguid(row) = guid.toLatin1().constData();
    if (!contains(guid)) {
        d->archiveView.Add(row);
        markDirty();
        setTotalCount(totalCount() + 1);
    }
}

bool FeedStorage::contains(const QString &guid) const
{
    return findArticle(guid) != -1;
}

int FeedStorage::findArticle(const QString &guid) const
{
    c4_Row findrow;
    d->pguid(findrow) = guid.toLatin1().constData();
    return d->archiveView.Find(findrow);
}

void FeedStorage::deleteArticle(const QString &guid)
{
    int findidx = findArticle(guid);
    if (findidx != -1) {
        setTotalCount(totalCount() - 1);
        d->archiveView.RemoveAt(findidx);
        markDirty();
    }
}

bool FeedStorage::guidIsHash(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsHash(d->archiveView.GetAt(findidx)) : false;
}

bool FeedStorage::guidIsPermaLink(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsPermaLink(d->archiveView.GetAt(findidx)) : false;
}

uint FeedStorage::hash(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->phash(d->archiveView.GetAt(findidx)) : 0;
}

void FeedStorage::setDeleted(const QString &guid)
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

QString FeedStorage::link(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(QByteArray(d->plink(d->archiveView.GetAt(findidx)))) : QLatin1StringView("");
}

QDateTime FeedStorage::pubDate(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QDateTime::fromSecsSinceEpoch(d->ppubDate(d->archiveView.GetAt(findidx))) : QDateTime();
}

int FeedStorage::status(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pstatus(d->archiveView.GetAt(findidx)) : 0;
}

void FeedStorage::setStatus(const QString &guid, int status)
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

void FeedStorage::article(const QString &guid, uint &hash, QString &title, int &status, QDateTime &pubDate) const
{
    int idx = findArticle(guid);
    if (idx != -1) {
        auto view = d->archiveView.GetAt(idx);
        hash = d->phash(view);
        title = QString::fromUtf8(QByteArray(d->ptitle(view)));
        status = d->pstatus(view);
        pubDate = QDateTime::fromSecsSinceEpoch(d->ppubDate(view));
    }
}

QString FeedStorage::title(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(QByteArray(d->ptitle(d->archiveView.GetAt(findidx)))) : QLatin1StringView("");
}

QString FeedStorage::description(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(QByteArray(d->pdescription(d->archiveView.GetAt(findidx)))) : QLatin1StringView("");
}

QString FeedStorage::content(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(QByteArray(d->pcontent(d->archiveView.GetAt(findidx)))) : QLatin1StringView("");
}

void FeedStorage::setPubDate(const QString &guid, const QDateTime &pubdate)
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

void FeedStorage::setGuidIsHash(const QString &guid, bool isHash)
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

void FeedStorage::setLink(const QString &guid, const QString &link)
{
    int findidx = findArticle(guid);
    if (findidx == -1) {
        return;
    }
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->plink(row) = !link.isEmpty() ? link.toUtf8().constData() : "";
    d->archiveView.SetAt(findidx, row);
    markDirty();
}

void FeedStorage::setHash(const QString &guid, uint hash)
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

void FeedStorage::setTitle(const QString &guid, const QString &title)
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

void FeedStorage::setDescription(const QString &guid, const QString &description)
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

void FeedStorage::setContent(const QString &guid, const QString &content)
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

void FeedStorage::setAuthorName(const QString &guid, const QString &author)
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

void FeedStorage::setAuthorUri(const QString &guid, const QString &author)
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

void FeedStorage::setAuthorEMail(const QString &guid, const QString &author)
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

QString FeedStorage::authorName(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(QByteArray(d->pauthorName(d->archiveView.GetAt(findidx)))) : QString();
}

QString FeedStorage::authorUri(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(QByteArray(d->pauthorUri(d->archiveView.GetAt(findidx)))) : QString();
}

QString FeedStorage::authorEMail(const QString &guid) const
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(QByteArray(d->pauthorEMail(d->archiveView.GetAt(findidx)))) : QString();
}

void FeedStorage::setGuidIsPermaLink(const QString &guid, bool isPermaLink)
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

void FeedStorage::setEnclosure(const QString &guid, const QString &url, const QString &type, int length)
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

void FeedStorage::removeEnclosure(const QString &guid)
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

void FeedStorage::enclosure(const QString &guid, bool &hasEnclosure, QString &url, QString &type, int &length) const
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
    url = QLatin1StringView(d->pEnclosureUrl(row));
    type = QLatin1StringView(d->pEnclosureType(row));
    length = d->pEnclosureLength(row);
}

void FeedStorage::setCategories(const QString &, const QStringList &categories)
{
    // TODO
}

QStringList FeedStorage::categories(const QString &guid) const
{
    // TODO
    return {};
}
} // namespace Backend
} // namespace Akregator
