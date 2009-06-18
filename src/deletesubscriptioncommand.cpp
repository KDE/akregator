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

#include "deletesubscriptioncommand.h"

#include "feed.h"
#include "feedlist.h"
#include "folder.h"
#include "subscriptionlistjobs.h"
#include "treenodevisitor.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QPointer>
#include <QTimer>

using namespace Akregator;
using namespace boost;

namespace {

class DeleteNodeVisitor : public TreeNodeVisitor
{
    public:
        explicit DeleteNodeVisitor( QWidget* parent ) : m_widget( parent ), m_job( 0 ) {}


        bool visitFolder( Folder* node )
        {
            const QString msg = node->title().isEmpty()
                                ? i18n("<qt>Are you sure you want to delete this folder and its feeds and subfolders?</qt>")
                                : i18n("<qt>Are you sure you want to delete folder <b>%1</b> and its feeds and subfolders?</qt>", node->title());

            if ( KMessageBox::warningContinueCancel( m_widget,
                                                     msg,
                                                     i18n( "Delete Folder" ),
                                                     KStandardGuiItem::del(),
                                                     KStandardGuiItem::cancel(),
                                                     "Disable delete folder confirmation" ) != KMessageBox::Continue )
                return true;
            m_job = reallyCreateJob( node );
            //TODO: port focus
            //m_widget->m_feedListView->setFocus();
            return true;
        }

        virtual bool visitFeed(Feed* node)
        {
            QString msg;
            if (node->title().isEmpty())
                msg = i18n("<qt>Are you sure you want to delete this feed?</qt>");
            else
                msg = i18n("<qt>Are you sure you want to delete feed <b>%1</b>?</qt>", node->title());

            if ( KMessageBox::warningContinueCancel( m_widget,
                                                     msg,
                                                     i18n( "Delete Feed" ),
                                                     KStandardGuiItem::del(),
                                                     KStandardGuiItem::cancel(),
                                                     "Disable delete feed confirmation" ) != KMessageBox::Continue )
                return true;
            m_job = reallyCreateJob( node );
            //TODO: port focus
            // m_widget->m_feedListView->setFocus();
            return true;
        }


        DeleteSubscriptionJob* createJob( TreeNode* node )
        {
            m_job = 0;
            if ( node )
                visit( node );
            return m_job;
        }

    private:
        static DeleteSubscriptionJob* reallyCreateJob( TreeNode* node )
        {
            DeleteSubscriptionJob* job = new DeleteSubscriptionJob;
            job->setSubscriptionId( node->id() );
            return job;
        }

    private:
        QPointer<QWidget> m_widget;
        QPointer<DeleteSubscriptionJob> m_job;
};

}

class DeleteSubscriptionCommand::Private
{
    DeleteSubscriptionCommand* const q;
public:
    explicit Private( DeleteSubscriptionCommand* qq );
    ~Private();

    void startDelete();
    void jobFinished();

    weak_ptr<FeedList> m_list;
    int m_subscriptionId;
};

DeleteSubscriptionCommand::Private::Private( DeleteSubscriptionCommand* qq ) : q( qq ),
                                                                               m_list(),
                                                                               m_subscriptionId( -1 )
{

}

DeleteSubscriptionCommand::Private::~Private()
{
}

DeleteSubscriptionCommand::DeleteSubscriptionCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{
}

DeleteSubscriptionCommand::~DeleteSubscriptionCommand()
{
    delete d;
}

void DeleteSubscriptionCommand::setSubscription( const weak_ptr<FeedList>& feedList, int subId )
{
    d->m_list = feedList;
    d->m_subscriptionId = subId;
}

int DeleteSubscriptionCommand::subscriptionId() const
{
    return d->m_subscriptionId;
}

weak_ptr<FeedList> DeleteSubscriptionCommand::feedList() const
{
    return d->m_list;
}

void DeleteSubscriptionCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT( startDelete() ) );
}

void DeleteSubscriptionCommand::Private::jobFinished()
{
    q->emitResult();
}

void DeleteSubscriptionCommand::Private::startDelete()
{
    const shared_ptr<FeedList> list = m_list.lock();
    if ( !list ) {
        q->emitResult();
        return;
    }
    TreeNode* const node = list->findByID( m_subscriptionId );
    DeleteNodeVisitor visitor( q->parentWidget() );
    DeleteSubscriptionJob* job = visitor.createJob( node );
    if ( !job )
    {
        q->emitResult();
        return;
    }

    QObject::connect( job, SIGNAL( finished( KJob* ) ), q, SLOT( jobFinished() ) );
    job->start();
}

#include "deletesubscriptioncommand.moc"
