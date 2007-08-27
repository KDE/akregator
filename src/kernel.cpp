/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include "kernel.h"

#include <k3staticdeleter.h>

#include "fetchqueue.h"
#include "framemanager.h"

namespace Akregator
{

Kernel* Kernel::m_self = 0;

static K3StaticDeleter<Kernel> kernelsd;

Kernel* Kernel::self()
{
    if (!m_self)
        m_self = kernelsd.setObject(m_self, new Kernel);
    return m_self;
}

class Kernel::KernelPrivate
{
    public:

    Backend::Storage* storage;
    FeedList* feedList;
    FetchQueue* fetchQueue;
    FrameManager* frameManager;
};

Kernel::Kernel() : d(new KernelPrivate)
{
    d->fetchQueue = new FetchQueue();
    d->frameManager = new FrameManager();
    d->storage = 0;
    d->feedList = 0;
}

Kernel::~Kernel()
{
    delete d->fetchQueue;
    delete d->frameManager;
    delete d;
    d = 0;
}

Backend::Storage* Kernel::storage() 
{ 
    return d->storage; 
}

void Kernel::setStorage(Backend::Storage* storage) 
{ 
    d->storage = storage;
}

FeedList* Kernel::feedList() 
{ 
    return d->feedList;
}

void Kernel::setFeedList(FeedList* feedList) 
{
    d->feedList = feedList;
}

FetchQueue* Kernel::fetchQueue()
{
    return d->fetchQueue; 
}

FrameManager* Kernel::frameManager()
{
    return d->frameManager;
}

} // namespace Akregator
