/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "storagefactoryregistry.h"
#include "storagefactory.h"

#include <QString>

namespace Akregator
{
namespace Backend
{
StorageFactoryRegistry *StorageFactoryRegistry::m_instance = nullptr;

StorageFactoryRegistry *StorageFactoryRegistry::self()
{
    static StorageFactoryRegistry instance;
    if (!m_instance) {
        m_instance = &instance;
    }
    return m_instance;
}

bool StorageFactoryRegistry::registerFactory(StorageFactory *factory, const QString &typestr)
{
    if (containsFactory(typestr)) {
        return false;
    }
    m_map[typestr] = factory;
    return true;
}

void StorageFactoryRegistry::unregisterFactory(const QString &typestr)
{
    m_map.remove(typestr);
}

StorageFactory *StorageFactoryRegistry::getFactory(const QString &typestr)
{
    return m_map[typestr];
}

bool StorageFactoryRegistry::containsFactory(const QString &typestr) const
{
    return m_map.contains(typestr);
}

QStringList StorageFactoryRegistry::list() const
{
    return m_map.keys();
}

StorageFactoryRegistry::StorageFactoryRegistry() = default;

StorageFactoryRegistry::~StorageFactoryRegistry()
{
    qDeleteAll(m_map);
}
} // namespace Backend
} // namespace Akregator
