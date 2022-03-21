/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "command.h"

#include <QWeakPointer>

#include <memory>

namespace Akregator
{
class FeedList;
class DeleteSubscriptionCommandPrivate;

class DeleteSubscriptionCommand : public Command
{
    Q_OBJECT
public:
    explicit DeleteSubscriptionCommand(QObject *parent = nullptr);
    ~DeleteSubscriptionCommand() override;

    void setSubscription(const QWeakPointer<FeedList> &feedList, uint subId);

    uint subscriptionId() const;
    QWeakPointer<FeedList> feedList() const;

private:
    void doStart() override;
    void doAbort() override;

private:
    friend class DeleteSubscriptionCommandPrivate;
    std::unique_ptr<DeleteSubscriptionCommandPrivate> const d;
};
}
