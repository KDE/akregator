/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregatorpart_export.h"
#include "storagefactory.h"
#include <QString>

#include <QStringList>

namespace Akregator
{
namespace Backend
{
class Storage;

class AKREGATORPART_EXPORT StorageFactoryDummyImpl : public StorageFactory
{
public:
    QString key() const override;
    QString name() const override;
    void configure() override;
    bool isConfigurable() const override
    {
        return false;
    }

    Storage *createStorage(const QStringList &params) const override;
};
} // namespace Backend
} // namespace Akregator

