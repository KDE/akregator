/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_KERNEL_H
#define AKREGATOR_KERNEL_H

#include <QSharedPointer>

#include "akregator_export.h"

namespace Akregator {
namespace Backend {
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

    Q_REQUIRED_RESULT QSharedPointer<FeedList> feedList() const;
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

#endif // AKREGATOR_KERNEL_H
