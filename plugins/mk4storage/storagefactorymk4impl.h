/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_BACKEND_STORAGEFACTORYMK4IMPL_H
#define AKREGATOR_BACKEND_STORAGEFACTORYMK4IMPL_H

#include "storagefactory.h"
#include <QString>
#include <QStringList>

namespace Akregator {
namespace Backend {
class Storage;

class StorageFactoryMK4Impl : public StorageFactory
{
public:
    QString key() const override;
    QString name() const override;
    void configure() override;
    Storage *createStorage(const QStringList &params) const override;
    bool isConfigurable() const override
    {
        return false;
    }
};
} // namespace Backend
} // namespace Akregator

#endif // AKREGATOR_BACKEND_STORAGEFACTORYMK4IMPL_H
