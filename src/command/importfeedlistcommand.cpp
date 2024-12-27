/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2009 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "importfeedlistcommand.h"

#include "feedlist.h"
#include "folder.h"
#include "kernel.h"

#include "akregator_debug.h"
#include <KLocalizedString>
#include <QInputDialog>

#include <QDomDocument>
#include <QPointer>
#include <QTimer>

#include <QSharedPointer>

#include <cassert>

using namespace Akregator;

class Akregator::ImportFeedListCommandPrivate
{
    ImportFeedListCommand *const q;

public:
    explicit ImportFeedListCommandPrivate(ImportFeedListCommand *qq);

    void doImport();

    QWeakPointer<FeedList> targetList;
    QDomDocument document;
    ImportFeedListCommand::RootFolderOption rootFolderOption;
    QString importedRootFolderName;
};

ImportFeedListCommandPrivate::ImportFeedListCommandPrivate(ImportFeedListCommand *qq)
    : q(qq)
    , targetList()
    , rootFolderOption(ImportFeedListCommand::RootFolderOption::Ask)
    , importedRootFolderName(i18n("Imported Feeds"))
{
}

void ImportFeedListCommandPrivate::doImport()
{
    const QSharedPointer<FeedList> target = targetList.lock();

    if (!target) {
        qCWarning(AKREGATOR_LOG) << "Target list was deleted, could not import feed list";
        q->done();
        return;
    }

    QScopedPointer<FeedList> importedList(new FeedList(Kernel::self()->storage()));
    const bool parsed = importedList->readFromOpml(document);

    // FIXME: parsing error, print some message
    if (!parsed) {
        q->done();
        return;
    }

    QPointer<QObject> that(q);

    bool ok = false;

    if (rootFolderOption == ImportFeedListCommand::RootFolderOption::Ask) {
        importedRootFolderName = QInputDialog::getText(q->parentWidget(),
                                                       i18n("Add Imported Folder"),
                                                       i18n("Imported folder name:"),
                                                       QLineEdit::Normal,
                                                       importedRootFolderName,
                                                       &ok);
    }

    if (!ok || !that) {
        if (that) {
            q->done();
        }
        return;
    }

    Folder *folder = target->allFeedsFolder();

    if (rootFolderOption != ImportFeedListCommand::RootFolderOption::None) {
        folder = new Folder(importedRootFolderName);
        target->allFeedsFolder()->appendChild(folder);
    }

    target->append(importedList.data(), folder);

    q->done();
}

ImportFeedListCommand::ImportFeedListCommand(QObject *parent)
    : Command(parent)
    , d(new ImportFeedListCommandPrivate(this))
{
}

ImportFeedListCommand::~ImportFeedListCommand() = default;

void ImportFeedListCommand::setTargetList(const QWeakPointer<FeedList> &feedList)
{
    d->targetList = feedList;
}

void ImportFeedListCommand::setImportedRootFolderOption(RootFolderOption opt)
{
    d->rootFolderOption = opt;
}

void ImportFeedListCommand::setImportedRootFolderName(const QString &defaultName)
{
    d->importedRootFolderName = defaultName;
}

void ImportFeedListCommand::setFeedListDocument(const QDomDocument &doc)
{
    d->document = doc;
}

void ImportFeedListCommand::doAbort()
{
    // TODO
}

void ImportFeedListCommand::doStart()
{
    QTimer::singleShot(0, this, [this]() {
        d->doImport();
    });
}

#include "moc_importfeedlistcommand.cpp"
