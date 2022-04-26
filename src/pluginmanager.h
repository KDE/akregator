/***************************************************************************
    SPDX-FileCopyrightText: 2004 Mark Kretschmann <markey@web.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later                             *
 *                                                                         *
 ***************************************************************************/

#pragma once

#include "akregatorpart_export.h"

#include <KService>

#include <vector>

namespace Akregator
{
class Plugin;
class AKREGATORPART_EXPORT PluginManager
{
public:
    /**
     * It will return a list of services that match your
     * specifications.  The only required parameter is the service
     * type.  This is something like 'text/plain' or 'text/html'.  The
     * constraint parameter is used to limit the possible choices
     * returned based on the constraints you give it.
     *
     * The @p constraint language is rather full.  The most common
     * keywords are AND, OR, NOT, IN, and EXIST, all used in an
     * almost spoken-word form.  An example is:
     * \code
     * (Type == 'Service') and (('KParts/ReadOnlyPart' in ServiceTypes) or (exist Exec))
     * \endcode
     *
     * The keys used in the query (Type, ServiceType, Exec) are all
     * fields found in the .desktop files.
     *
     * @param constraint  A constraint to limit the choices returned, QString() to
     *                    get all services of the given @p servicetype
     *
     * @return            A list of services that satisfy the query
     * @see               http://developer.kde.org/documentation/library/kdeqt/tradersyntax.html
     */
    static KService::List query(const QString &constraint = QString());

    /**
     * Load and instantiate plugin from service
     * @param service     Pointer to KService
     * @param parent      Parent object
     * @return            Pointer to Plugin, or NULL if error
     */
    static Akregator::Plugin *createFromService(const KService::Ptr &service, QObject *parent = nullptr);

    /**
     * Dump properties from a service to stdout for debugging
     * @param service     Pointer to KService
     */
    static void dump(const KService::Ptr &service);

private:
    struct StoreItem {
        Akregator::Plugin *plugin = nullptr;
        KService::Ptr service;
    };

    // attributes:
    static std::vector<StoreItem> m_store;
};
} // namespace Akregator
