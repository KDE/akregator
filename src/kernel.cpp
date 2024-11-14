/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kernel.h"
#include "config-akregator.h"

#include "feedlist.h"
#include "fetchqueue.h"
#include "framemanager.h"
#if AKREGATOR_WITH_KUSERFEEDBACK
#include "userfeedback/userfeedbackmanager.h"
#endif
#if HAVE_ACTIVITY_SUPPORT
#include "activities/activitiesmanager.h"
#endif

using namespace Akregator;

Kernel *Kernel::m_self = nullptr;

Kernel *Kernel::self()
{
    static Kernel self;
    if (!m_self) {
        m_self = &self;
    }
    return m_self;
}

Kernel::Kernel()
    : m_fetchQueue(new FetchQueue())
    , m_frameManager(new FrameManager())
#if HAVE_ACTIVITY_SUPPORT
    , mActivitiesManager(new ActivitiesManager())
#endif
{
#if AKREGATOR_WITH_KUSERFEEDBACK
    // Initialize
    (void)UserFeedBackManager::self();
#endif
}

Kernel::~Kernel()
{
    delete m_fetchQueue;
    delete m_frameManager;
#if HAVE_ACTIVITY_SUPPORT
    delete mActivitiesManager;
#endif
}

#if HAVE_ACTIVITY_SUPPORT
ActivitiesManager *Kernel::activitiesManager() const
{
    return mActivitiesManager;
}
#endif

Backend::Storage *Kernel::storage()
{
    return m_storage;
}

void Kernel::setStorage(Backend::Storage *storage)
{
    m_storage = storage;
}

QSharedPointer<FeedList> Kernel::feedList() const
{
    return m_feedList;
}

void Kernel::setFeedList(const QSharedPointer<FeedList> &feedList)
{
    m_feedList = feedList;
}

FetchQueue *Kernel::fetchQueue() const
{
    return m_fetchQueue;
}

FrameManager *Kernel::frameManager() const
{
    return m_frameManager;
}
