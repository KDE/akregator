/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "command.h"

#include <QSharedPointer>

class QDomDocument;

namespace Akregator
{
namespace Backend
{
class Storage;
}

class FeedList;

class LoadFeedListCommand : public Command
{
    Q_OBJECT
public:
    explicit LoadFeedListCommand(QObject *parent = nullptr);
    ~LoadFeedListCommand() override;

    void setFileName(const QString &fileName);
    void setDefaultFeedList(const QDomDocument &doc);
    void setStorage(Backend::Storage *storage);

Q_SIGNALS:
    void result(const QSharedPointer<Akregator::FeedList> &feedList);

private:
    void doStart() override;
    void doAbort() override;

private:
    class Private;
    Private *const d;
};
}

