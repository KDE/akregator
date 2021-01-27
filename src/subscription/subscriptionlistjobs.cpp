/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "subscriptionlistjobs.h"
#include "feedlist.h"
#include "folder.h"
#include "kernel.h"
#include "treenode.h"

#include <KLocalizedString>

#include <QDebug>
#include <QTimer>

using namespace Akregator;

MoveSubscriptionJob::MoveSubscriptionJob(QObject *parent)
    : KJob(parent)
    , m_id(0)
    , m_destFolderId(0)
    , m_afterId(-1)
    , m_feedList(Kernel::self()->feedList())
{
}

void MoveSubscriptionJob::setSubscriptionId(int id)
{
    m_id = id;
}

void MoveSubscriptionJob::setDestination(int folder, int afterChild)
{
    m_destFolderId = folder;
    m_afterId = afterChild;
}

void MoveSubscriptionJob::start()
{
    QTimer::singleShot(20, this, &MoveSubscriptionJob::doMove);
}

void MoveSubscriptionJob::doMove()
{
    const QSharedPointer<FeedList> feedList = m_feedList.lock();

    if (!feedList) {
        setErrorText(i18n("Feed list was deleted"));
        emitResult();
        return;
    }

    TreeNode *const node = feedList->findByID(m_id);
    auto *const destFolder = qobject_cast<Folder *>(feedList->findByID(m_destFolderId));
    TreeNode *const after = feedList->findByID(m_afterId);

    if (!node || !destFolder) {
        setErrorText(i18n("Node or destination folder not found"));
        emitResult();
        return;
    }
    const Folder *const asFolder = qobject_cast<Folder *>(node);

    if (asFolder && asFolder->subtreeContains(destFolder)) {
        setErrorText(i18n("Cannot move folder %1 to its own subfolder %2", asFolder->title(), destFolder->title()));
        emitResult();
        return;
    }

    node->parent()->removeChild(node);
    if (after) {
        destFolder->insertChild(node, after);
    } else {
        destFolder->appendChild(node);
    }
    emitResult();
}

RenameSubscriptionJob::RenameSubscriptionJob(QObject *parent)
    : KJob(parent)
    , m_id(0)
    , m_feedList(Kernel::self()->feedList())
{
}

void RenameSubscriptionJob::setSubscriptionId(int id)
{
    m_id = id;
}

void RenameSubscriptionJob::setName(const QString &name)
{
    m_name = name;
}

void RenameSubscriptionJob::start()
{
    QTimer::singleShot(20, this, &RenameSubscriptionJob::doRename);
}

void RenameSubscriptionJob::doRename()
{
    if (m_id > 0) {
        TreeNode *const node = m_feedList->findByID(m_id);
        if (node) {
            node->setTitle(m_name);
        }
    }
    emitResult();
}

DeleteSubscriptionJob::DeleteSubscriptionJob(QObject *parent)
    : KJob(parent)
    , m_id(0)
    , m_feedList(Kernel::self()->feedList())
{
}

void DeleteSubscriptionJob::setSubscriptionId(int id)
{
    m_id = id;
}

void DeleteSubscriptionJob::start()
{
    QTimer::singleShot(20, this, &DeleteSubscriptionJob::doDelete);
}

void DeleteSubscriptionJob::doDelete()
{
    const QSharedPointer<FeedList> feedList = m_feedList.lock();
    if (feedList) {
        delete feedList->findByID(m_id);
    }
    emitResult();
}
