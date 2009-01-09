/*
    This file is part of Akregator.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

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

#include "subscriptionlistjobs.h"
#include "feedlist.h"
#include "folder.h"
#include "kernel.h"
#include "treenode.h"

#include <KLocalizedString>

#include <QTimer>

using namespace boost;
using namespace Akregator;

MoveSubscriptionJob::MoveSubscriptionJob( QObject* parent ) : KJob( parent ), m_id( 0 ), m_destFolderId( 0 ), m_afterId( -1 ), m_feedList( Kernel::self()->feedList() )
{
}

void MoveSubscriptionJob::setSubscriptionId( int id )
{
    m_id = id;
}

void MoveSubscriptionJob::setDestination( int folder, int afterChild )
{
    m_destFolderId = folder;
    m_afterId = afterChild;
}


void MoveSubscriptionJob::start()
{
    QTimer::singleShot( 20, this, SLOT( doMove() ) );
}

void MoveSubscriptionJob::doMove()
{
    const shared_ptr<FeedList> feedList = m_feedList.lock();

    if ( !feedList ) {
        setErrorText( i18n( "Feed list was deleted" ) );
        emitResult();
        return;
    }

    TreeNode* const node = feedList->findByID( m_id );
    Folder* const destFolder = qobject_cast<Folder*>( feedList->findByID( m_destFolderId ) );
    TreeNode* const after = feedList->findByID( m_afterId );

    if ( !node || !destFolder )
    {
        setErrorText( i18n( "Node or destination folder not found" ) );
        emitResult();
        return;
    }
    const Folder* const asFolder = qobject_cast<Folder*>( node );

    if ( asFolder && asFolder->subtreeContains( destFolder ) )
    {
        setErrorText( i18n( "Cannot move folder %1 to its own subfolder %2", asFolder->title(), destFolder->title() ) );
        emitResult();
        return;
    }

    node->parent()->removeChild( node );
    if ( after )
        destFolder->insertChild( node, after );
    else
        destFolder->appendChild( node );
    emitResult();
}

RenameSubscriptionJob::RenameSubscriptionJob( QObject* parent ) : KJob( parent ), m_id( 0 ), m_feedList( Kernel::self()->feedList() )
{
}

void RenameSubscriptionJob::setSubscriptionId( int id )
{
    m_id = id;
}

void RenameSubscriptionJob::setName( const QString& name )
{
    m_name = name;
}

void RenameSubscriptionJob::start()
{
    QTimer::singleShot( 20, this, SLOT( doRename() ) );
}

void RenameSubscriptionJob::doRename()
{
    if ( m_id > 0 )
    {
        TreeNode* const node = m_feedList->findByID( m_id );
        if ( node )
            node->setTitle( m_name );
    }
    emitResult();
}

DeleteSubscriptionJob::DeleteSubscriptionJob( QObject* parent ) : KJob( parent ), m_id( 0 ), m_feedList( Kernel::self()->feedList() )
{
}

void DeleteSubscriptionJob::setSubscriptionId( int id )
{
    m_id = id;
}

void DeleteSubscriptionJob::start()
{
    QTimer::singleShot( 20, this, SLOT( doDelete() ) );
}

void DeleteSubscriptionJob::doDelete()
{
    const shared_ptr<FeedList> feedList = m_feedList.lock();
    if ( m_id > 0 && feedList )
        delete feedList->findByID( m_id );
    emitResult();
}

#include "subscriptionlistjobs.moc"
