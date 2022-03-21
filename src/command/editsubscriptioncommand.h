/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "command.h"

#include <QSharedPointer>

#include <memory>

namespace Akregator
{
class FeedList;
class SubscriptionListView;
class EditSubscriptionCommandPrivate;

class EditSubscriptionCommand : public Command
{
    Q_OBJECT
public:
    explicit EditSubscriptionCommand(QObject *parent = nullptr);
    ~EditSubscriptionCommand() override;

    void setSubscription(const QSharedPointer<FeedList> &feedList, uint subId);
    uint subscriptionId() const;
    QSharedPointer<FeedList> feedList() const;

    SubscriptionListView *subscriptionListView() const;
    void setSubscriptionListView(SubscriptionListView *view);

private:
    void doStart() override;
    void doAbort() override;

private:
    friend class EditSubscriptionCommandPrivate;
    std::unique_ptr<EditSubscriptionCommandPrivate> const d;
};
}
