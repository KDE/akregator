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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_BACKEND_STORAGEFACTORY_H
#define AKREGATOR_BACKEND_STORAGEFACTORY_H

class QString;
class QStringList;
class QWidget;
namespace Akregator {
namespace Backend {

class Storage;

class StorageFactory
{
    public:
    
    /** identifier of the storage type, like "metakit", "postgres" etc. For use in
        configuration files. Must not contain spaces or special characters.
     */
    virtual QString key() const = 0;
    
    /** returns the (i18n'd) name of the storage type. */
    virtual QString name() const = 0;

    /** true if the plugin is configurable via a config dialog */
    virtual bool isConfigurable() const = 0;
    
    /** shows the plugin's configuration dialog */
    virtual void configure() = 0;

    /**
     * returns wether the backend allows multiple writers at the same time
     * If not, Akregator must use a lock to ensure that only one process gains
     * write access.
     */
    virtual bool allowsMultipleWriteAccess() const = 0;
    
    /** creates a storage object with given parameters
        @param params list of implementation-specific parameters
     */
	virtual Storage* createStorage(const QStringList& params) const = 0;
};

}
}

#endif
