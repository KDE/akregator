/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QSharedPointer>

#include "akregator_export.h"

namespace Akregator
{
namespace Backend
{
class Storage;
}

class FeedList;
class FetchQueue;
class FrameManager;

class AKREGATOR_EXPORT Kernel
{
public:
    static Kernel *self();

    ~Kernel();

    Backend::Storage *storage();
    void setStorage(Backend::Storage *storage);

    [[nodiscard]] QSharedPointer<FeedList> feedList() const;
    void setFeedList(const QSharedPointer<FeedList> &feedList);

    FetchQueue *fetchQueue() const;

    FrameManager *frameManager() const;

private:
    Kernel();

    static Kernel *m_self;

    Backend::Storage *m_storage = nullptr;
    QSharedPointer<FeedList> m_feedList;
    FetchQueue *m_fetchQueue = nullptr;
    FrameManager *m_frameManager = nullptr;
};
} // namespace Akregator
