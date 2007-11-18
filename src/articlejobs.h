/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_ARTICLE_JOBS_H
#define AKREGATOR_ARTICLE_JOBS_H

#include <KJob>

#include <QList>
#include <QMap>
#include <QString>

//transitional job classes
namespace Akregator {

class FeedList;

struct ArticleId
{
    QString feedUrl;
    QString guid;
    bool operator<( const ArticleId& other ) const
    {
        return feedUrl < other.feedUrl || ( feedUrl == other.feedUrl  && guid < other.guid );
    }
};

typedef QList<Akregator::ArticleId> ArticleIdList;

class ArticleDeleteJob : public KJob
{
Q_OBJECT
public:
    explicit ArticleDeleteJob( QObject* parent = 0 );

    void appendArticleIds( const Akregator::ArticleIdList& ids );
    void appendArticleId( const Akregator::ArticleId& id );

    void start();

private:
    Akregator::FeedList* m_feedList;
    ArticleIdList m_ids;
};

class ArticleModifyJob : public KJob
{
public:
    explicit ArticleModifyJob( QObject* parent = 0 );

    // TODO replace this by passing modified item later
    void setStatus( const ArticleId& id, int status );
    void setKeep( const ArticleId& id, bool keep );

    void start();

private:
    Akregator::FeedList* m_feedList;
    QMap<ArticleId, bool> m_keepFlags;
    QMap<ArticleId, int> m_status;
};

} // namespace akregator

#endif // AKREGATOR_ARTICLE_JOBS_H
