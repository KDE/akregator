/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "command.h"

#include <memory>

namespace Akregator
{
class Folder;
class MainWidget;
class SubscriptionListView;
class TreeNode;
class CreateFeedCommandPrivate;

class CreateFeedCommand : public Command
{
    Q_OBJECT
public:
    explicit CreateFeedCommand(MainWidget *parent = nullptr);
    ~CreateFeedCommand() override;

    void setSubscriptionListView(SubscriptionListView *view);
    void setRootFolder(Folder *rootFolder);
    void setUrl(const QString &url);
    void setPosition(Folder *parent, TreeNode *after);
    void setAutoExecute(bool autoexec);

private:
    void doStart() override;
    void doAbort() override;

private:
    friend class CreateFeedCommandPrivate;
    std::unique_ptr<CreateFeedCommandPrivate> const d;
};
}

