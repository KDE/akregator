/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "mk4plugin.h"

#include "storagefactorymk4impl.h"
#include "storagefactoryregistry.h"

namespace Akregator {
namespace Backend {
K_PLUGIN_FACTORY(MK4PluginFactory,
                 registerPlugin<MK4Plugin>();
                 )

void MK4Plugin::doInitialize()
{
    m_factory = new StorageFactoryMK4Impl();
    StorageFactoryRegistry::self()->registerFactory(m_factory, QStringLiteral("metakit"));
}

MK4Plugin::MK4Plugin(QObject *parent, const QVariantList &params) : Plugin(parent, params)
{
}

MK4Plugin::~MK4Plugin()
{
    StorageFactoryRegistry::self()->unregisterFactory(QStringLiteral("metakit"));
    delete m_factory;
}
} // namespace Backend
} // namespace Akregator
#include "mk4plugin.moc"
