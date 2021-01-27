/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_BACKEND_STORAGEFACTORYREGISTRY_H
#define AKREGATOR_BACKEND_STORAGEFACTORYREGISTRY_H

#include "akregatorinterfaces_export.h"
#include <QHash>
class QString;
#include <QStringList>

namespace Akregator
{
namespace Backend
{
class StorageFactory;

class AKREGATORINTERFACES_EXPORT StorageFactoryRegistry
{
public:
    static StorageFactoryRegistry *self();

    ~StorageFactoryRegistry();

    bool registerFactory(StorageFactory *factory, const QString &typestr);
    void unregisterFactory(const QString &typestr);
    StorageFactory *getFactory(const QString &typestr);
    bool containsFactory(const QString &typestr) const;
    QStringList list() const;

private:
    static StorageFactoryRegistry *m_instance;

    StorageFactoryRegistry();
    StorageFactoryRegistry(const StorageFactoryRegistry &);
    StorageFactoryRegistry &operator=(const StorageFactoryRegistry &);

    QHash<QString, StorageFactory *> m_map;
};
} // namespace Backend
} // namespace Akregator

#endif // STORAGEFACTORYREGISTRY_H
