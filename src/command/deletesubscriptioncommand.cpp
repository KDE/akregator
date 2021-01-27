/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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

namespace
{
class DeleteNodeVisitor : public TreeNodeVisitor
{
public:
    explicit DeleteNodeVisitor(QWidget *parent)
        : m_widget(parent)
        , m_job(nullptr)
    {
    }

    bool visitFolder(Folder *node) override
    {
        const QString msg = node->title().isEmpty()
            ? i18n("<qt>Are you sure you want to delete this folder and its feeds and subfolders?</qt>")
            : i18n("<qt>Are you sure you want to delete folder <b>%1</b> and its feeds and subfolders?</qt>", node->title());

        if (KMessageBox::warningContinueCancel(m_widget,
                                               msg,
                                               i18n("Delete Folder"),
                                               KStandardGuiItem::del(),
                                               KStandardGuiItem::cancel(),
                                               QStringLiteral("Disable delete folder confirmation"))
            != KMessageBox::Continue) {
            return true;
        }
        m_job = reallyCreateJob(node);
        // TODO: port focus
        // m_widget->m_feedListView->setFocus();
        return true;
    }

    bool visitFeed(Akregator::Feed *node) override
    {
        QString msg;
        if (node->title().isEmpty()) {
            msg = i18n("<qt>Are you sure you want to delete this feed?</qt>");
        } else {
            msg = i18n("<qt>Are you sure you want to delete feed <b>%1</b>?</qt>", node->title());
        }

        if (KMessageBox::warningContinueCancel(m_widget,
                                               msg,
                                               i18n("Delete Feed"),
                                               KStandardGuiItem::del(),
                                               KStandardGuiItem::cancel(),
                                               QStringLiteral("Disable delete feed confirmation"))
            != KMessageBox::Continue) {
            return true;
        }
        m_job = reallyCreateJob(node);
        // TODO: port focus
        // m_widget->m_feedListView->setFocus();
        return true;
    }

    DeleteSubscriptionJob *createJob(TreeNode *node)
    {
        m_job = nullptr;
        if (node) {
            visit(node);
        }
        return m_job;
    }

private:
    static DeleteSubscriptionJob *reallyCreateJob(TreeNode *node)
    {
        auto *job = new DeleteSubscriptionJob;
        job->setSubscriptionId(node->id());
        return job;
    }

private:
    QPointer<QWidget> m_widget;
    QPointer<DeleteSubscriptionJob> m_job;
};
}

class DeleteSubscriptionCommand::Private
{
    DeleteSubscriptionCommand *const q;

public:
    explicit Private(DeleteSubscriptionCommand *qq);
    ~Private();

    void startDelete();
    void jobFinished();

    QWeakPointer<FeedList> m_list;
    int m_subscriptionId = -1;
};

DeleteSubscriptionCommand::Private::Private(DeleteSubscriptionCommand *qq)
    : q(qq)
    , m_list()
{
}

DeleteSubscriptionCommand::Private::~Private()
{
}

DeleteSubscriptionCommand::DeleteSubscriptionCommand(QObject *parent)
    : Command(parent)
    , d(new Private(this))
{
}

DeleteSubscriptionCommand::~DeleteSubscriptionCommand()
{
    delete d;
}

void DeleteSubscriptionCommand::setSubscription(const QWeakPointer<FeedList> &feedList, int subId)
{
    d->m_list = feedList;
    d->m_subscriptionId = subId;
}

int DeleteSubscriptionCommand::subscriptionId() const
{
    return d->m_subscriptionId;
}

QWeakPointer<FeedList> DeleteSubscriptionCommand::feedList() const
{
    return d->m_list;
}

void DeleteSubscriptionCommand::doStart()
{
    QTimer::singleShot(0, this, [this]() {
        d->startDelete();
    });
}

void DeleteSubscriptionCommand::Private::jobFinished()
{
    q->done();
}

void DeleteSubscriptionCommand::Private::startDelete()
{
    const QSharedPointer<FeedList> list = m_list.lock();
    if (!list) {
        q->done();
        return;
    }
    TreeNode *const node = list->findByID(m_subscriptionId);
    DeleteNodeVisitor visitor(q->parentWidget());
    DeleteSubscriptionJob *job = visitor.createJob(node);
    if (!job) {
        q->done();
        return;
    }

    QObject::connect(job, &DeleteSubscriptionJob::finished, q, [this]() {
        jobFinished();
    });
    job->start();
}

void DeleteSubscriptionCommand::doAbort()
{
}

#include "moc_deletesubscriptioncommand.cpp"
