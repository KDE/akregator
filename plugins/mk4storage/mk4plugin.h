/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "plugin.h"

#include <KPluginFactory>

namespace Akregator {
namespace Backend {
class StorageFactory;

class MK4Plugin : public Akregator::Plugin
{
    Q_OBJECT
public:
    explicit MK4Plugin(QObject *parent, const QVariantList &params);
    ~MK4Plugin() override;

private:
    void doInitialize() override;
    StorageFactory *m_factory = nullptr;
};
} // namespace Backend
} // namespace Akregator

