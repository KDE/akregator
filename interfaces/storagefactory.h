/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregatorinterfaces_export.h"

class QString;
#include <QStringList>
namespace Akregator
{
namespace Backend
{
class Storage;

class AKREGATORINTERFACES_EXPORT StorageFactory
{
public:
    virtual ~StorageFactory() = default;

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

    /** creates a storage object with given parameters
        @param params list of implementation-specific parameters
     */
    virtual Storage *createStorage(const QStringList &params) const = 0;
};
} // namespace Backend
} // namespace Akregator

