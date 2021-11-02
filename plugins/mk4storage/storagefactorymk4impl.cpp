/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "storagefactorymk4impl.h"
#include "storagemk4impl.h"

#include <KLocalizedString>

namespace Akregator {
namespace Backend {
Storage *StorageFactoryMK4Impl::createStorage(const QStringList &params) const
{
    Storage *storage = new StorageMK4Impl;
    storage->initialize(params);
    return storage;
}

QString StorageFactoryMK4Impl::key() const
{
    return QStringLiteral("metakit");
}

QString StorageFactoryMK4Impl::name() const
{
    return i18n("Metakit");
}

void StorageFactoryMK4Impl::configure()
{
}
} // namespace Backend
} // namespace Akregator
