/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_EXPIREITEMSCOMMAND_H
#define AKREGATOR_EXPIREITEMSCOMMAND_H

#include "command.h"

#include <QVector>

#include <QWeakPointer>

namespace Akregator
{
class FeedList;

class ExpireItemsCommand : public Command
{
    Q_OBJECT
public:
    explicit ExpireItemsCommand(QObject *parent = nullptr);
    ~ExpireItemsCommand() override;

    void setFeedList(const QWeakPointer<FeedList> &feedList);
    QWeakPointer<FeedList> feedList() const;

    void setFeeds(const QVector<uint> &feeds);
    QVector<uint> feeds() const;

private:
    void doStart() override;
    void doAbort() override;

private:
    class Private;
    Private *const d;
};
}

#endif // AKREGATOR_EXPIREITEMSCOMMAND_H
