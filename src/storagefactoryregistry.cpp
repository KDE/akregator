/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "storagefactory.h"
#include "storagefactoryregistry.h"

#include <kstaticdeleter.h>

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

namespace Akregator {
namespace Backend {

class StorageFactoryRegistry::StorageFactoryRegistryPrivate
{
    public:
        QMap<QString, StorageFactory*> map;
};

StorageFactoryRegistry* StorageFactoryRegistry::m_instance = 0;
static KStaticDeleter<StorageFactoryRegistry> storagefactoryregistrysd;

StorageFactoryRegistry* StorageFactoryRegistry::self()
{
    if (!m_instance)
        m_instance = storagefactoryregistrysd.setObject(m_instance, new StorageFactoryRegistry);
    return m_instance;
}
	
bool StorageFactoryRegistry::registerFactory(StorageFactory* factory, const QString& typestr)
{
    if (containsFactory(typestr))
        return false;
    d->map[typestr] = factory;
    return true; 
}

void StorageFactoryRegistry::unregisterFactory(const QString& typestr)
{
    d->map.remove(typestr);
}

StorageFactory* StorageFactoryRegistry::getFactory(const QString& typestr)
{
    return d->map[typestr];
}

bool StorageFactoryRegistry::containsFactory(const QString& typestr) const
{
    return d->map.contains(typestr);
}

QStringList StorageFactoryRegistry::list() const
{
    return d->map.keys();
}

StorageFactoryRegistry::StorageFactoryRegistry() : d(new StorageFactoryRegistryPrivate)
{
}

StorageFactoryRegistry::~StorageFactoryRegistry()
{
    delete d; 
    d = 0;
}

}
}
