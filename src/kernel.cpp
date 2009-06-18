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

#include "kernel.h"
#include "framemanager.h"

#include <krss/feedlist.h>

#include <k3staticdeleter.h>

#include <QPointer>

using namespace boost;
using namespace KRss;

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

    shared_ptr<KRss::FeedList> feedList;
    FrameManager* frameManager;
};

Kernel::Kernel() : d(new KernelPrivate)
{
    d->frameManager = new FrameManager();
}

Kernel::~Kernel()
{
    delete d->frameManager;
    delete d;
    d = 0;
}


shared_ptr<FeedList> Kernel::feedList() const
{
    return d->feedList;
}

void Kernel::setFeedList(const shared_ptr<FeedList>& feedList)
{
    d->feedList = feedList;
}

FrameManager* Kernel::frameManager()
{
    return d->frameManager;
}

} // namespace Akregator
