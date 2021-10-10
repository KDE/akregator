/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2009 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "command.h"

#include <QWeakPointer>

#include <memory>

class QDomDocument;

namespace Akregator
{
class FeedList;
class ImportFeedListCommandPrivate;

class ImportFeedListCommand : public Command
{
    Q_OBJECT
public:
    explicit ImportFeedListCommand(QObject *parent = nullptr);
    ~ImportFeedListCommand() override;

    void setTargetList(const QWeakPointer<FeedList> &feedList);

    enum RootFolderOption { None, Auto, Ask };

    void setImportedRootFolderOption(RootFolderOption opt);
    void setImportedRootFolderName(const QString &defaultName);

    void setFeedListDocument(const QDomDocument &doc);

private:
    void doStart() override;
    void doAbort() override;

private:
    friend class ImportFeedListCommandPrivate;
    std::unique_ptr<ImportFeedListCommandPrivate> const d;
};
}

