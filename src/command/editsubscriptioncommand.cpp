/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "editsubscriptioncommand.h"

#include "feed.h"
#include "feedlist.h"
#include "feedpropertiesdialog.h"
#include "folder.h"
#include "subscriptionlistview.h"
#include "treenodevisitor.h"

#include <QPointer>
#include <QTimer>

using namespace Akregator;

namespace
{
class EditNodePropertiesVisitor : public TreeNodeVisitor
{
public:
    EditNodePropertiesVisitor(SubscriptionListView *subcriptionListView, QWidget *parent);

    bool visitFolder(Folder *node) override
    {
        m_subscriptionListView->startNodeRenaming(node);
        return true;
    }

    bool visitFeed(Akregator::Feed *node) override
    {
        QPointer<FeedPropertiesDialog> dlg = new FeedPropertiesDialog(m_widget);
        dlg->setFeed(node);
        dlg->exec();
        delete dlg;
        return true;
    }

private:
    SubscriptionListView *m_subscriptionListView = nullptr;
    QWidget *m_widget = nullptr;
};
}

EditNodePropertiesVisitor::EditNodePropertiesVisitor(SubscriptionListView *subscriptionListView, QWidget *parent)
    : m_subscriptionListView(subscriptionListView)
    , m_widget(parent)
{
    Q_ASSERT(m_subscriptionListView);
    Q_ASSERT(m_widget);
}

class Akregator::EditSubscriptionCommandPrivate
{
    EditSubscriptionCommand *const q;

public:
    explicit EditSubscriptionCommandPrivate(EditSubscriptionCommand *qq);
    ~EditSubscriptionCommandPrivate();

    void startEdit();
    void jobFinished();

    QSharedPointer<FeedList> m_list;
    uint m_subscriptionId;
    SubscriptionListView *m_subscriptionListView = nullptr;
};

EditSubscriptionCommandPrivate::EditSubscriptionCommandPrivate(EditSubscriptionCommand *qq)
    : q(qq)
    , m_list()
    , m_subscriptionId(0)
    , m_subscriptionListView(nullptr)
{
}

EditSubscriptionCommandPrivate::~EditSubscriptionCommandPrivate() = default;

EditSubscriptionCommand::EditSubscriptionCommand(QObject *parent)
    : Command(parent)
    , d(new EditSubscriptionCommandPrivate(this))
{
}

EditSubscriptionCommand::~EditSubscriptionCommand() = default;

void EditSubscriptionCommand::setSubscription(const QSharedPointer<FeedList> &feedList, uint subId)
{
    d->m_list = feedList;
    d->m_subscriptionId = subId;
}

uint EditSubscriptionCommand::subscriptionId() const
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
    QTimer::singleShot(0, this, [this]() {
        d->startEdit();
    });
}

void EditSubscriptionCommandPrivate::startEdit()
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
