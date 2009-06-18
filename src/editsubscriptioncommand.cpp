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

#include "editsubscriptioncommand.h"

#include <krss/feedjobs.h>
#include <krss/feedlist.h>
#include <krss/feedvisitor.h>
#include <krss/netfeed.h>
#include <krss/tag.h>
#include <krss/tagjobs.h>
#include <krss/tagprovider.h>
#include <krss/treenode.h>
#include <krss/treenodevisitor.h>
#include <krss/ui/feedlistview.h>
#include <krss/ui/feedpropertiesdialog.h>
#include <krss/ui/tagpropertiesdialog.h>

#include <KDebug>
#include <KLocalizedString>
#include <KMessageBox>

#include <QPointer>
#include <QTimer>

#include <cassert>

using namespace Akregator;
using namespace boost;
using namespace KRss;

class EditSubscriptionCommand::Private : public TreeNodeVisitor, public FeedVisitor
{
    EditSubscriptionCommand* const q;
public:
    explicit Private( EditSubscriptionCommand* qq );
    ~Private();

    void visit( const shared_ptr<RootNode>& node )
    {
        q->emitResult();
    }

    void visit( const shared_ptr<TagNode>& node )
    {
        Tag tag = node->tag();
        QPointer<QObject> that( q );
        QPointer<TagPropertiesDialog> dialog( new TagPropertiesDialog( q->parentWidget() ) );
        dialog->setLabel( tag.label() );
        dialog->setDescription( tag.description() );
        if ( dialog->exec() == KDialog::Accepted  ) {
            tag.setLabel( dialog->label() );
            tag.setDescription( dialog->description() );
            TagModifyJob * const job = tagProvider->tagModifyJob();
            job->setTag( tag );
            connect( job, SIGNAL(finished(KJob*)), q, SLOT(tagModifyDone(KJob*)) );
            job->start();
        } else {
            if ( that )
                q->emitResult();
        }
        delete dialog;
    }

    void visit( const shared_ptr<FeedNode>& node )
    {
        const shared_ptr<Feed> feed = feedList->feedById( node->feedId() );
        if ( !feed ) {
            q->emitResult();
            return;
        }
        feed->accept( this );
    }

    void visitNetFeed( const shared_ptr<NetFeed>& nf ) {
        QPointer<QObject> that( q );
        QPointer<FeedPropertiesDialog> dlg( new FeedPropertiesDialog( q->parentWidget() ) );
        dlg->setFeedTitle( nf->title() );
        dlg->setUrl( nf->xmlUrl() );
        dlg->setCustomFetchInterval( nf->fetchInterval() > 0 ); //PENDING(frank) correct?
        dlg->setFetchInterval( nf->fetchInterval() );

        if ( dlg->exec() != QDialog::Accepted ) {
            delete dlg;
            if ( that )
                q->emitResult();
            return;
        }
        nf->setTitle( dlg->feedTitle() );
        nf->setXmlUrl( dlg->url() );
        nf->setFetchInterval( dlg->hasCustomFetchInterval() ? dlg->fetchInterval() : 0 );
        FeedModifyJob* job = new FeedModifyJob( nf );
        connect( job, SIGNAL(finished(KJob*)), q, SLOT(feedModifyDone(KJob*)) );
        job->start();
    }

    void feedModifyDone( KJob* job ) {
        if ( job->error() )
            KMessageBox::error( q->parentWidget(), i18n("Could not save feed settings: %1", job->errorString() ) );
        q->emitResult();
    }

    void tagModifyDone( KJob* job ) {
        if ( job->error() )
            KMessageBox::error( q->parentWidget(), i18n("Could not save tag settings: %1", job->errorString() ) );
        q->emitResult();
    }

    void startEdit();
    void jobFinished();

    shared_ptr<const TagProvider> tagProvider;
    shared_ptr<TreeNode> node;
    shared_ptr<FeedList> feedList;
    QPointer<FeedListView> feedListView;
};

EditSubscriptionCommand::Private::Private( EditSubscriptionCommand* qq ) : q( qq ),
                                                                               node(),
                                                                               feedListView()
{

}

EditSubscriptionCommand::Private::~Private()
{
}

EditSubscriptionCommand::EditSubscriptionCommand( QObject* parent ) : Command( parent ), d( new Private( this ) )
{
}

EditSubscriptionCommand::~EditSubscriptionCommand()
{
    delete d;
}

void EditSubscriptionCommand::setNode( const shared_ptr<TreeNode>& node )
{
    d->node = node;
}

shared_ptr<TreeNode> EditSubscriptionCommand::node() const
{
    return d->node;
}

FeedListView* EditSubscriptionCommand::feedListView() const
{
    return d->feedListView;
}

void EditSubscriptionCommand::setFeedListView( FeedListView* view )
{
    d->feedListView = view;
}

void EditSubscriptionCommand::doStart()
{
    QTimer::singleShot( 0, this, SLOT(startEdit()) );
}

void EditSubscriptionCommand::Private::startEdit()
{
    if ( !node ) {
        q->emitResult();
        return;
    }
    QPointer<QObject> that( q );
    node->accept( this );
}

shared_ptr<FeedList> EditSubscriptionCommand::feedList() const {
    return d->feedList;
}

void EditSubscriptionCommand::setFeedList( const shared_ptr<FeedList>& fl ) {
    d->feedList = fl;
}

shared_ptr<const TagProvider> EditSubscriptionCommand::tagProvider() const
{
    return d->tagProvider;
}

void EditSubscriptionCommand::setTagProvider( const shared_ptr<const TagProvider>& tp )
{
    d->tagProvider = tp;
}

#include "editsubscriptioncommand.moc"
