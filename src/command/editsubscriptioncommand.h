/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_EDITSUBSCRIPTIONCOMMAND_H
#define AKREGATOR_EDITSUBSCRIPTIONCOMMAND_H

#include "command.h"

#include <QSharedPointer>

namespace Akregator
{
class FeedList;
class SubscriptionListView;

class EditSubscriptionCommand : public Command
{
    Q_OBJECT
public:
    explicit EditSubscriptionCommand(QObject *parent = nullptr);
    ~EditSubscriptionCommand() override;

    void setSubscription(const QSharedPointer<FeedList> &feedList, int subId);
    int subscriptionId() const;
    QSharedPointer<FeedList> feedList() const;

    SubscriptionListView *subscriptionListView() const;
    void setSubscriptionListView(SubscriptionListView *view);

private:
    void doStart() override;
    void doAbort() override;

private:
    class Private;
    Private *const d;
};
}

#endif // AKREGATOR_EDITSUBSCRIPTIONCOMMAND_H
