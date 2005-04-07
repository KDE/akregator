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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "feedstoragemk4impl.h"
#include "storagemk4impl.h"

#include "../myarticle.h"
#include "../librss/article.h"
#include "../librss/document.h"
#include <mk4.h>

#include <qdatetime.h>
#include <qdom.h>
#include <qfile.h>

#include <kglobal.h>
#include <kstandarddirs.h>

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
            phash("hash"),
            pguidIsHash("guidIsHash"),
            pguidIsPermaLink("guidIsPermaLink"),
            pcomments("comments"),
            pstatus("status"),
            ppubDate("pubDate")
        {}
            
        QString url;
        c4_Storage* storage;
        StorageMK4Impl* mainStorage;
        c4_View archiveView;
        bool autoCommit;
	bool modified;
        bool convert;
        QString oldArchivePath;
        c4_StringProp pguid, ptitle, pdescription, plink, pcommentsLink;
        c4_IntProp phash, pguidIsHash, pguidIsPermaLink, pcomments, pstatus, ppubDate;
};

void FeedStorageMK4Impl::convertOldArchive()
{
    if (!d->convert)
        return;

    d->convert = false;    
    QFile file(d->oldArchivePath);

    if ( !file.open(IO_ReadOnly) )
        return;
    
    QTextStream stream(&file);
    stream.setEncoding(QTextStream::UnicodeUTF8);
    QString data=stream.read();
    QDomDocument xmldoc;
    
    if (!xmldoc.setContent(data))
        return;
    
    RSS::Document doc(xmldoc);

    RSS::Article::List d_articles = doc.articles();
    RSS::Article::List::ConstIterator it = d_articles.begin();
    RSS::Article::List::ConstIterator en = d_articles.end();
    
    int unr = 0;
    for (; it != en; ++it)
    {
        MyArticle a(*it, this);
        if (a.status() != MyArticle::Read)
            unr++;
    }
    
    setUnread(unr);
    d->modified = true;
    commit();
}

FeedStorageMK4Impl::FeedStorageMK4Impl(const QString& url, StorageMK4Impl* main)
{
    d = new FeedStorageMK4ImplPrivate;
    d->autoCommit = main->autoCommit();
    d->url = url;
    d->mainStorage = main;
    QString t = url;
    QString t2 = url;
    QString filePath = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + t.replace("/", "_").replace(":", "_") + ".mk4";
    d->oldArchivePath = KGlobal::dirs()->saveLocation("data", "akregator/Archive/") + t2.replace("/", "_").replace(":", "_") + ".xml";
    d->convert = !QFile::exists(filePath) && QFile::exists(d->oldArchivePath);
    d->storage = new c4_Storage(filePath.local8Bit(), true);
    d->archiveView = d->storage->GetAs("articles[guid:S,title:S,hash:I,guidIsHash:I,guidIsPermaLink:I,description:S,link:S,comments:I,commentsLink:S,status:I,pubDate:I]");
    c4_View hash = d->storage->GetAs("archiveHash[_H:I,_R:I]");
    d->archiveView = d->archiveView.Hash(hash, 1); // hash on guid
}

FeedStorageMK4Impl::~FeedStorageMK4Impl()
{
    // TODO: close etc.
    delete d->storage;
    delete d; d = 0;
}

void FeedStorageMK4Impl::commit()
{
    if (d->modified)
        d->storage->Commit();
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
int FeedStorageMK4Impl::unread()
{
    return d->mainStorage->unreadFor(d->url);
}
void FeedStorageMK4Impl::setUnread(int unread)
{
    d->mainStorage->setUnreadFor(d->url, unread);
}

int FeedStorageMK4Impl::totalCount()
{
    return d->mainStorage->totalCountFor(d->url);
}

void FeedStorageMK4Impl::setTotalCount(int total)
{
    d->mainStorage->setTotalCountFor(d->url, total);
}

int FeedStorageMK4Impl::lastFetch()
{
    return d->mainStorage->lastFetchFor(d->url);
}

void FeedStorageMK4Impl::setLastFetch(int lastFetch)
{
    d->mainStorage->setLastFetchFor(d->url, lastFetch);
}

QStringList FeedStorageMK4Impl::articles()
{
    QStringList list;
    int size = d->archiveView.GetSize();
    for (int i = 0; i < size; i++)
        list += QString(d->pguid(d->archiveView.GetAt(i)));
    // fill with guids
    return list;
}

void FeedStorageMK4Impl::addEntry(const QString& guid)
{
    c4_Row row;
    d->pguid(row) = guid.ascii();
    if (!contains(guid))
    {
        d->archiveView.Add(row);
	d->modified = true;
        setTotalCount(totalCount()+1);
    }
}

bool FeedStorageMK4Impl::contains(const QString& guid)
{
    return findArticle(guid) != -1;
}

int FeedStorageMK4Impl::findArticle(const QString& guid)
{
    c4_Row findrow;
    d->pguid(findrow) = guid.ascii();
    return d->archiveView.Find(findrow);
}

void FeedStorageMK4Impl::deleteArticle(const QString& guid)
{

    int findidx = findArticle(guid);
    if (findidx != -1)
    {
        setTotalCount(totalCount()-1);
        d->archiveView.RemoveAt(findidx);
	d->modified = true;
    }   
}

int FeedStorageMK4Impl::comments(const QString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pcomments(d->archiveView.GetAt(findidx)) : 0;
}

QString FeedStorageMK4Impl::commentsLink(const QString& guid)
{
   int findidx = findArticle(guid);
   return findidx != -1 ? QString(d->pcommentsLink(d->archiveView.GetAt(findidx))) : "";
}

bool FeedStorageMK4Impl::guidIsHash(const QString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsHash(d->archiveView.GetAt(findidx)) : false;
}

bool FeedStorageMK4Impl::guidIsPermaLink(const QString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? d->pguidIsPermaLink(d->archiveView.GetAt(findidx)) : false;
}

uint FeedStorageMK4Impl::hash(const QString& guid)
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
    d->pdescription(row) = "";
    d->ptitle(row) = "";
    d->plink(row) = "";
    d->pcommentsLink(row) = "";
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

QString FeedStorageMK4Impl::link(const QString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString(d->plink(d->archiveView.GetAt(findidx))) : "";
}

QDateTime FeedStorageMK4Impl::pubDate(const QString& guid)
{
    int findidx = findArticle(guid);
    QDateTime time;
    if (findidx != -1)
        time.setTime_t(d->ppubDate(d->archiveView.GetAt(findidx)));
    return time;
}

int FeedStorageMK4Impl::status(const QString& guid)
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

QString FeedStorageMK4Impl::title(const QString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->ptitle(d->archiveView.GetAt(findidx))) : "";
}

QString FeedStorageMK4Impl::description(const QString& guid)
{
    int findidx = findArticle(guid);
    return findidx != -1 ? QString::fromUtf8(d->pdescription(d->archiveView.GetAt(findidx))) : "";
}


void FeedStorageMK4Impl::setPubDate(const QString& guid, const QDateTime& pubdate)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->ppubDate(row) = pubdate.toTime_t();
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
    d->plink(row) = !link.isEmpty() ? link.ascii() : "";
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
    d->ptitle(row) = !title.isEmpty() ? title.utf8().data() : "";
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
    d->pdescription(row) = !description.isEmpty() ? description.utf8().data() : "";
    d->archiveView.SetAt(findidx, row);
    d->modified = true;
}

void FeedStorageMK4Impl::setCommentsLink(const QString& guid, const QString& commentsLink)
{
    int findidx = findArticle(guid);
    if (findidx == -1)
        return;
    c4_Row row;
    row = d->archiveView.GetAt(findidx);
    d->pcommentsLink(row) = !commentsLink.isEmpty() ? commentsLink.utf8().data() : "";
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

}
}
