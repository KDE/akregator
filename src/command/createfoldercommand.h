/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "command.h"

namespace Akregator
{
class Folder;
class SubscriptionListView;
class TreeNode;

class CreateFolderCommand : public Command
{
    Q_OBJECT
public:
    explicit CreateFolderCommand(QObject *parent = nullptr);
    ~CreateFolderCommand() override;

    void setSubscriptionListView(SubscriptionListView *view);
    void setSelectedSubscription(TreeNode *selected);
    void setRootFolder(Folder *rootFolder);

private:
    void doStart() override;
    void doAbort() override;

private:
    class Private;
    Private *const d;
};
}

