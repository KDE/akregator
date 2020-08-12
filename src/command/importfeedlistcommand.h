/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2009 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_IMPORTFEEDLISTCOMMAND_H
#define AKREGATOR_IMPORTFEEDLISTCOMMAND_H

#include "command.h"

#include <QWeakPointer>

class QDomDocument;

namespace Akregator {
class FeedList;

class ImportFeedListCommand : public Command
{
    Q_OBJECT
public:
    explicit ImportFeedListCommand(QObject *parent = nullptr);
    ~ImportFeedListCommand() override;

    void setTargetList(const QWeakPointer<FeedList> &feedList);

    enum RootFolderOption {
        None,
        Auto,
        Ask
    };

    void setImportedRootFolderOption(RootFolderOption opt);
    void setImportedRootFolderName(const QString &defaultName);

    void setFeedListDocument(const QDomDocument &doc);

private:
    void doStart() override;
    void doAbort() override;

private:
    class Private;
    Private *const d;
};
}

#endif // AKREGATOR_IMPORTFEEDLISTCOMMAND_H
