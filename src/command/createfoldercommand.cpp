/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "createfoldercommand.h"

#include "folder.h"
#include "subscriptionlistview.h"

#include <QInputDialog>
#include <KLocalizedString>

#include <QTimer>

#include <cassert>

using namespace Akregator;

class Q_DECL_HIDDEN CreateFolderCommand::Private
{
    CreateFolderCommand *const q;
public:
    explicit Private(CreateFolderCommand *qq);

    void doCreate();

    TreeNode *m_selectedSubscription = nullptr;
    Folder *m_rootFolder = nullptr;
    SubscriptionListView *m_subscriptionListView = nullptr;
};

CreateFolderCommand::Private::Private(CreateFolderCommand *qq)
    : q(qq)
{
}

void CreateFolderCommand::Private::doCreate()
{
    Q_ASSERT(m_rootFolder);
    Q_ASSERT(m_subscriptionListView);
    bool ok;
    const QString name = QInputDialog::getText(q->parentWidget(), i18n("Add Folder"),
                                               i18n("Folder name:"), QLineEdit::Normal,
                                               QString(),
                                               &ok);
    if (!ok || name.trimmed().isEmpty()) {
        q->done();
        return;
    }

    Folder *parentFolder = qobject_cast<Folder *>(m_selectedSubscription);
    if (!parentFolder) {
        parentFolder = m_selectedSubscription ? m_selectedSubscription->parent() : nullptr;
    }
    if (!parentFolder) {
        parentFolder = m_rootFolder;
    }

    TreeNode *const after = (m_selectedSubscription && m_selectedSubscription->isGroup()) ? m_selectedSubscription : nullptr;

    Folder *const newFolder = new Folder(name);
    parentFolder->insertChild(newFolder, after);
    m_subscriptionListView->ensureNodeVisible(newFolder);
    q->done();
}

CreateFolderCommand::CreateFolderCommand(QObject *parent) : Command(parent)
    , d(new Private(this))
{
}

CreateFolderCommand::~CreateFolderCommand()
{
    delete d;
}

void CreateFolderCommand::setSubscriptionListView(SubscriptionListView *view)
{
    d->m_subscriptionListView = view;
}

void CreateFolderCommand::setSelectedSubscription(TreeNode *selected)
{
    d->m_selectedSubscription = selected;
}

void CreateFolderCommand::setRootFolder(Folder *rootFolder)
{
    d->m_rootFolder = rootFolder;
}

void CreateFolderCommand::doStart()
{
    QTimer::singleShot(0, this, [this]() {
        d->doCreate();
    });
}

void CreateFolderCommand::doAbort()
{
}

#include "moc_createfoldercommand.cpp"
