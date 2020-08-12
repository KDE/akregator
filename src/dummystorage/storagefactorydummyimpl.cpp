/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "storagefactorydummyimpl.h"
#include "storagedummyimpl.h"

#include <KLocalizedString>
#include <QString>
#include <QStringList>

namespace Akregator {
namespace Backend {
Storage *StorageFactoryDummyImpl::createStorage(const QStringList &params) const
{
    Storage *storage = new StorageDummyImpl;
    storage->initialize(params);
    return storage;
}

QString StorageFactoryDummyImpl::key() const
{
    return QStringLiteral("dummy");
}

QString StorageFactoryDummyImpl::name() const
{
    return i18n("No Archive");
}

void StorageFactoryDummyImpl::configure()
{
}
}
}
