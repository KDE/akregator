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

#include "feed.h"
#include "feedlist.h"
#include "folder.h"
#include "feedpropertiesdialog.h"
#include "subscriptionlistview.h"
#include "treenodevisitor.h"

#include <QTimer>

#include <cassert>

using namespace Akregator;

namespace
{

class EditNodePropertiesVisitor : public TreeNodeVisitor
{
public:
    EditNodePropertiesVisitor(SubscriptionListView *subcriptionListView, QWidget *parent);

    bool visitFolder(Folder *node)
    {
        m_subscriptionListView->startNodeRenaming(node);
        return true;
    }

    bool visitFeed(Akregator::Feed *node)
    {
        QPointer<FeedPropertiesDialog> dlg = new FeedPropertiesDialog(m_widget);
        dlg->setFeed(node);
        dlg->exec();
        delete dlg;
        return true;
    }
private:

    SubscriptionListView *m_subscriptionListView;
    QWidget *m_widget;
};

}

EditNodePropertiesVisitor::EditNodePropertiesVisitor(SubscriptionListView *subscriptionListView, QWidget *parent) : m_subscriptionListView(subscriptionListView), m_widget(parent)
{
    Q_ASSERT(m_subscriptionListView);
    Q_ASSERT(m_widget);
}

class EditSubscriptionCommand::Private
{
    EditSubscriptionCommand *const q;
public:
    explicit Private(EditSubscriptionCommand *qq);
    ~Private();

    void startEdit();
    void jobFinished();

    QSharedPointer<FeedList> m_list;
    int m_subscriptionId;
    SubscriptionListView *m_subscriptionListView;
};

EditSubscriptionCommand::Private::Private(EditSubscriptionCommand *qq) : q(qq),
    m_list(),
    m_subscriptionId(-1),
    m_subscriptionListView(0)
{

}

EditSubscriptionCommand::Private::~Private()
{
}

EditSubscriptionCommand::EditSubscriptionCommand(QObject *parent) : Command(parent), d(new Private(this))
{
}

EditSubscriptionCommand::~EditSubscriptionCommand()
{
    delete d;
}

void EditSubscriptionCommand::setSubscription(const QSharedPointer<FeedList> &feedList, int subId)
{
    d->m_list = feedList;
    d->m_subscriptionId = subId;
}

int EditSubscriptionCommand::subscriptionId() const
{
    return d->m_subscriptionId;
}

QSharedPointer<FeedList> EditSubscriptionCommand::feedList() const
{
    return d->m_list;
}

SubscriptionListView *EditSubscriptionCommand::subscriptionListView() const
{
    return d->m_subscriptionListView;
}

void EditSubscriptionCommand::setSubscriptionListView(SubscriptionListView *view)
{
    d->m_subscriptionListView = view;
}

void EditSubscriptionCommand::doStart()
{
    QTimer::singleShot(0, this, SLOT(startEdit()));
}

void EditSubscriptionCommand::Private::startEdit()
{
    TreeNode *const node = m_list->findByID(m_subscriptionId);
    if (!node) {
        q->done();
        return;
    }

    EditNodePropertiesVisitor visitor(m_subscriptionListView, q->parentWidget());
    visitor.visit(node);
    q->done();
}

void EditSubscriptionCommand::doAbort()
{

}

#include "moc_editsubscriptioncommand.cpp"
