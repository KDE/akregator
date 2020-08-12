/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "createfeedcommand.h"

#include "addfeeddialog.h"
#include "feed.h"
#include "feedlist.h"
#include "feedpropertiesdialog.h"
#include "folder.h"
#include "mainwidget.h"
#include "subscriptionlistview.h"

#include <QUrl>

#include <QPointer>
#include <QTimer>
#include <QClipboard>

#include <cassert>

using namespace Akregator;

class Q_DECL_HIDDEN CreateFeedCommand::Private
{
    CreateFeedCommand *const q;
public:
    explicit Private(CreateFeedCommand *qq);

    void doCreate();

    QPointer<MainWidget> m_parent;
    QPointer<Folder> m_rootFolder;
    QPointer<SubscriptionListView> m_subscriptionListView;
    QString m_url;
    QPointer<Folder> m_parentFolder;
    QPointer<TreeNode> m_after;
    bool m_autoexec = false;
};

CreateFeedCommand::Private::Private(CreateFeedCommand *qq)
    : q(qq)
    , m_rootFolder(nullptr)
    , m_subscriptionListView(nullptr)
    , m_parentFolder(nullptr)
    , m_after(nullptr)
    , m_autoexec(false)
{
}

void CreateFeedCommand::Private::doCreate()
{
    Q_ASSERT(m_rootFolder);
    Q_ASSERT(m_subscriptionListView);

    QPointer<AddFeedDialog> afd = new AddFeedDialog(q->parentWidget(), QStringLiteral("add_feed"));

    QString url = m_url;

    if (url.isEmpty()) {
        const QClipboard *const clipboard = QApplication::clipboard();
        Q_ASSERT(clipboard);
        const QString clipboardText = clipboard->text();
        // Check for the hostname, since the isValid method is not strict enough
        if (!QUrl(clipboardText).host().isEmpty()) {
            url = clipboardText;
        }
    }

    afd->setUrl(QUrl::fromPercentEncoding(QUrl::fromUserInput(url).toEncoded()));

    QPointer<QObject> thisPointer(q);

    if (m_autoexec) {
        afd->accept();
    } else {
        afd->exec();
    }

    if (!thisPointer) {   // "this" might have been deleted while exec()!
        delete afd;
        return;
    }

    Feed *const feed = afd->feed();
    delete afd;

    if (!feed) {
        q->done();
        return;
    }

    QPointer<FeedPropertiesDialog> dlg = new FeedPropertiesDialog(q->parentWidget(), QStringLiteral("edit_feed"));
    dlg->setFeed(feed);
    dlg->selectFeedName();

    if (!m_autoexec && (dlg->exec() != QDialog::Accepted || !thisPointer)) {
        delete feed;
    } else {
        if (!m_parentFolder) {
            if (!m_rootFolder) {
                if (m_parent->allFeedsList()) {
                    q->setRootFolder(m_parent->allFeedsList()->allFeedsFolder());
                }
            }
            m_parentFolder = m_rootFolder;
        }

        if (m_parentFolder) {
            m_parentFolder->insertChild(feed, m_after);
            m_subscriptionListView->ensureNodeVisible(feed);
        }
    }

    delete dlg;
    q->done();
}

CreateFeedCommand::CreateFeedCommand(MainWidget *parent) : Command(parent)
    , d(new Private(this))
{
    d->m_parent = parent;
}

CreateFeedCommand::~CreateFeedCommand()
{
    delete d;
}

void CreateFeedCommand::setSubscriptionListView(SubscriptionListView *view)
{
    d->m_subscriptionListView = view;
}

void CreateFeedCommand::setRootFolder(Folder *rootFolder)
{
    d->m_rootFolder = rootFolder;
}

void CreateFeedCommand::setUrl(const QString &url)
{
    d->m_url = url;
}

void CreateFeedCommand::setPosition(Folder *parent, TreeNode *after)
{
    d->m_parentFolder = parent;
    d->m_after = after;
}

void CreateFeedCommand::setAutoExecute(bool autoexec)
{
    d->m_autoexec = autoexec;
}

void CreateFeedCommand::doStart()
{
    QTimer::singleShot(0, this, [this]() {
        d->doCreate();
    });
}

void CreateFeedCommand::doAbort()
{
}

#include "moc_createfeedcommand.cpp"
