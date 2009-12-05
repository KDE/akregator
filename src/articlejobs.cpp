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

#include <KDebug>
#include <KLocalizedString>

#include <QTimer>

#include <vector>

#include <cassert>

using namespace Akregator;

Akregator::ArticleDeleteJob::ArticleDeleteJob( QObject* parent ) : KJob( parent ), m_feedList( Kernel::self()->feedList() )
{
    assert( m_feedList );
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
    QTimer::singleShot( 20, this, SLOT( doStart() ) );
}

void Akregator::ArticleDeleteJob::doStart()
{
    if ( !m_feedList )
    {
        kWarning() << "Feedlist object was deleted, items not deleted";
        emitResult();
        return;
    }
    std::vector<Akregator::Feed*> feeds;

    Q_FOREACH ( const Akregator::ArticleId& id, m_ids )
    {
        Akregator::Article article = m_feedList->findArticle( id.feedUrl, id.guid );
        if ( article.isNull() )
            continue;

        if ( Feed* const feed = m_feedList->findByURL( id.feedUrl ) )
        {
            feeds.push_back( feed );
            feed->setNotificationMode( false );
        }
        article.setDeleted();
    }

    Q_FOREACH ( Akregator::Feed* const i, feeds )
        i->setNotificationMode( true );

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
    QTimer::singleShot( 20, this, SLOT( doStart() ) );
}

void Akregator::ArticleModifyJob::doStart()
{

    if ( !m_feedList )
    {
        kWarning() << "Feedlist object was deleted, items not modified";
        emitResult();
        return;
    }
    std::vector<Akregator::Feed*> feeds;

    Q_FOREACH ( const Akregator::ArticleId& id, m_keepFlags.keys() )
    {
        Akregator::Feed* feed = m_feedList->findByURL( id.feedUrl );
        if ( !feed )
            continue;
        feed->setNotificationMode( false );
        feeds.push_back( feed );
        Akregator::Article article = feed->findArticle( id.guid );
        if ( !article.isNull() )
            article.setKeep( m_keepFlags[id] );
    }

    Q_FOREACH ( const Akregator::ArticleId& id, m_status.keys() )
    {
        Akregator::Feed* feed = m_feedList->findByURL( id.feedUrl );
        if ( !feed )
            continue;
        feed->setNotificationMode( false );
        feeds.push_back( feed );
        Akregator::Article article = feed->findArticle( id.guid );
        if ( !article.isNull() )
            article.setStatus( m_status[id] );
    }

    Q_FOREACH ( Akregator::Feed* const i, feeds )
        i->setNotificationMode( true );
    emitResult();
}

CompositeJob::CompositeJob( QObject* parent ) : KCompositeJob( parent )
{

}

bool CompositeJob::addSubjob( KJob* job )
{
    return KCompositeJob::addSubjob( job );
}


void CompositeJob::start()
{
    if ( subjobs().isEmpty() )
    {
        emitResult();
        return;
    }
    Q_FOREACH( KJob* const i, subjobs() )
    {
        i->start();
    }
}

ArticleListJob::ArticleListJob( TreeNode* p ) : KJob( p ), m_node( p ) {}

void ArticleListJob::start() {
    QTimer::singleShot( 20, this, SLOT( doList() ) );
}

void ArticleListJob::doList() {
    if ( m_node )
        m_articles = m_node->articles();
    else {
        setError( ListingFailed );
        setErrorText( i18n("The feed to be listed was already removed.") );
    }
    emitResult();
}

TreeNode* ArticleListJob::node() const {
    return m_node;
}

QList<Article> ArticleListJob::articles() const {
    return m_articles;
}

#include "articlejobs.moc"
