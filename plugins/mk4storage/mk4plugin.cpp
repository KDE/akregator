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

#include "mk4plugin.h"

#include "storagefactorymk4impl.h"
#include "storagefactoryregistry.h"

namespace Akregator {
namespace Backend {

K_PLUGIN_FACTORY(MK4PluginFactory,
                 registerPlugin<MK4Plugin>();
)

K_EXPORT_PLUGIN(MK4PluginFactory("akregator_mk4storage"))

void MK4Plugin::doInitialize()
{
   m_factory = new StorageFactoryMK4Impl();
   StorageFactoryRegistry::self()->registerFactory(m_factory, QLatin1String("metakit"));
}

MK4Plugin::MK4Plugin( QObject* parent, const QVariantList& params ) : Plugin( parent, params ), m_factory( 0 ) {
}

MK4Plugin::~MK4Plugin()
{
    StorageFactoryRegistry::self()->unregisterFactory(QLatin1String("metakit"));
    delete m_factory;
}

} // namespace Backend
} // namespace Akregator
