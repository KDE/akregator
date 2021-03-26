/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_SUBSCRIPTIONLISTJOBS_H
#define AKREGATOR_SUBSCRIPTIONLISTJOBS_H

#include <KJob>

#include <QWeakPointer>

#include "akregator_export.h"

namespace Akregator
{
class FeedList;

// transitional job classes

class AKREGATOR_EXPORT MoveSubscriptionJob : public KJob
{
    Q_OBJECT
public:
    explicit MoveSubscriptionJob(QObject *parent = nullptr);

    void setSubscriptionId(uint id);
    void setDestination(uint folder, uint afterChild);

    void start() override;

private Q_SLOTS:
    void doMove();

private:
    uint m_id = 0;
    uint m_destFolderId = 0;
    uint m_afterId = 0;
    QWeakPointer<FeedList> m_feedList;
};

class AKREGATOR_EXPORT RenameSubscriptionJob : public KJob
{
    Q_OBJECT
public:
    explicit RenameSubscriptionJob(QObject *parent = nullptr);

    void setSubscriptionId(uint id);
    void setName(const QString &name);

    void start() override;

private Q_SLOTS:
    void doRename();

private:
    uint m_id;
    QString m_name;
    QSharedPointer<FeedList> m_feedList;
};

class AKREGATOR_EXPORT DeleteSubscriptionJob : public KJob
{
    Q_OBJECT
public:
    explicit DeleteSubscriptionJob(QObject *parent = nullptr);

    void setSubscriptionId(uint id);

    void start() override;

private:
    void doDelete();
    uint m_id;
    QWeakPointer<FeedList> m_feedList;
};
}
#endif // AKREGATOR_SUBSCRIPTIONLISTJOBS_H
