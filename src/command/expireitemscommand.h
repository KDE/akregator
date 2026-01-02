/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "command.h"

#include <QList>

#include <QWeakPointer>

#include <memory>

namespace Akregator
{
class FeedList;
class ExpireItemsCommandPrivate;

class ExpireItemsCommand : public Command
{
    Q_OBJECT
public:
    explicit ExpireItemsCommand(QObject *parent = nullptr);
    ~ExpireItemsCommand() override;

    void setFeedList(const QWeakPointer<FeedList> &feedList);
    [[nodiscard]] QWeakPointer<FeedList> feedList() const;

    void setFeeds(const QList<uint> &feeds);
    [[nodiscard]] QList<uint> feeds() const;

private:
    void doStart() override;
    void doAbort() override;
    friend class ExpireItemsCommandPrivate;
    std::unique_ptr<ExpireItemsCommandPrivate> const d;
};
}
