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

#include "articlejobs.h"
#include "article.h"
#include "feed.h"
#include "feedlist.h"
#include "kernel.h"

#include <QSet>

Akregator::ArticleDeleteJob::ArticleDeleteJob( QObject* parent ) : KJob( parent ), m_feedList( Kernel::self()->feedList() )
{
    Q_ASSERT( m_feedList );
}

void Akregator::ArticleDeleteJob::appendArticleIds( const QList<Akregator::ArticleId>& ids )
{
    m_ids += ids;
}

void Akregator::ArticleDeleteJob::appendArticleId( const Akregator::ArticleId& id )
{
    m_ids += id;
}

void Akregator::ArticleDeleteJob::start()
{
    Q_FOREACH ( const Akregator::ArticleId id, m_ids )
    {
        const Akregator::Feed* feed = m_feedList->findByURL( id.feedUrl );
        if ( !feed )
            continue;
        Akregator::Article article = feed->findArticle( id.guid );
        if ( !article.isNull() )
            article.setDeleted();
    } 

    emitResult();
}

Akregator::ArticleModifyJob::ArticleModifyJob( QObject* parent ) : KJob( parent ), m_feedList( Kernel::self()->feedList() )
{
    Q_ASSERT( m_feedList );
}

void Akregator::ArticleModifyJob::setStatus( const ArticleId& id, int status )
{
    m_status[id] = status;
}

void Akregator::ArticleModifyJob::setKeep( const ArticleId& id, bool keep )
{
    m_keepFlags[id] = keep;
}

void Akregator::ArticleModifyJob::start()
{
    QSet<Akregator::Feed*> feeds;

    Q_FOREACH ( const Akregator::ArticleId id, m_keepFlags.keys() )
    {
        Akregator::Feed* feed = m_feedList->findByURL( id.feedUrl );
        feed->setNotificationMode( false );
        feeds.insert( feed );
        if ( !feed )
            continue;
        Akregator::Article article = feed->findArticle( id.guid );
        if ( !article.isNull() )
            article.setKeep( m_keepFlags[id] );
    }

    Q_FOREACH ( const Akregator::ArticleId id, m_status.keys() )
    {
        Akregator::Feed* feed = m_feedList->findByURL( id.feedUrl );
        if ( !feed )
            continue;
        Akregator::Article article = feed->findArticle( id.guid );
        if ( !article.isNull() )
            article.setStatus( m_status[id] );
    }

    Q_FOREACH ( Akregator::Feed* i, feeds )
        i->setNotificationMode( true ); 
    emitResult();
}

#include "articlejobs.moc"
