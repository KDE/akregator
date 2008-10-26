/***************************************************************************
begin                : 2004/03/12
copyright            : (C) Mark Kretschmann
email                : markey@web.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "plugin.h"
#include "pluginmanager.h"

#include <vector>
#include <QFile>
#include <QString>

#include <klibloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

using std::vector;
using Akregator::Plugin;

namespace Akregator {

vector<PluginManager::StoreItem>
PluginManager::m_store;


/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC INTERFACE
/////////////////////////////////////////////////////////////////////////////////////

KService::List
PluginManager::query( const QString& constraint )
{
    // Add versioning constraint
    QString
    str  = "[X-KDE-akregator-framework-version] == ";
    str += QString::number( AKREGATOR_PLUGIN_INTERFACE_VERSION );
    str += " and ";
    if (!constraint.trimmed().isEmpty())
        str += constraint + " and ";
    str += "[X-KDE-akregator-rank] > 0";

    kDebug() <<"Plugin trader constraint:" << str;

    return KServiceTypeTrader::self()->query( "Akregator/Plugin", str );
}


Plugin*
PluginManager::createFromQuery( const QString &constraint )
{
    KService::List offers = query( constraint );

    if ( offers.isEmpty() ) {
        kWarning() <<"No matching plugin found.";
        return 0;
    }

    // Select plugin with highest rank
    int rank = 0;
    uint current = 0;
    for ( int i = 0; i < offers.count(); i++ ) {
        if ( offers[i]->property( "X-KDE-akregator-rank" ).toInt() > rank )
            current = i;
    }

    return createFromService( offers[current] );
}


Plugin*
PluginManager::createFromService( const KService::Ptr service )
{
    kDebug() <<"Trying to load:" << service->library();

    //get the library loader instance
    KLibLoader *loader = KLibLoader::self();
    //try to load the specified library
    KLibrary *lib = loader->library( QFile::encodeName( service->library() ), QLibrary::ExportExternalSymbolsHint );

    if ( !lib ) {
        KMessageBox::error( 0, i18n( "<p>KLibLoader could not load the plugin:<br/><i>%1</i></p>"
                                     "<p>Error message:<br/><i>%2</i></p>" ,
                                 service->library() ,
                                 loader->lastErrorMessage() ) );
        return 0;
    }
    //look up address of init function and cast it to pointer-to-function
    Plugin* (*create_plugin)() = ( Plugin* (*)() ) lib->resolveFunction( "create_plugin" );

    if ( !create_plugin ) {
        kWarning() <<"create_plugin == NULL";
        return 0;
    }
    //create plugin on the heap
    Plugin* plugin = create_plugin();

    //put plugin into store
    StoreItem item;
    item.plugin = plugin;
    item.library = lib;
    item.service = service;
    m_store.push_back( item );

    dump( service );
    return plugin;
}


void
PluginManager::unload( Plugin* plugin )
{
    vector<StoreItem>::iterator iter = lookupPlugin( plugin );

    if ( iter != m_store.end() ) {
        delete (*iter).plugin;
        kDebug() <<"Unloading library:"<< (*iter).service->library();
        (*iter).library->unload();

        m_store.erase( iter );
    }
    else
        kWarning() <<"Could not unload plugin (not found in store).";
}


KService::Ptr
PluginManager::getService( const Plugin* plugin )
{
    if ( !plugin ) {
        kWarning() <<"pointer == NULL";
        return KService::Ptr( 0 );
    }

    //search plugin in store
    vector<StoreItem>::const_iterator iter = lookupPlugin( plugin );

    if ( iter == m_store.end() ) {
        kWarning() <<"Plugin not found in store.";
        return KService::Ptr( 0 );
    }

    return (*iter).service;
}


void
PluginManager::showAbout( const QString &constraint )
{
    KService::List offers = query( constraint );

    if ( offers.isEmpty() )
        return;

    KService::Ptr s = offers.front();

    const QString body = "<tr><td>%1</td><td>%2</td></tr>";

    QString str  = "<html><body><table width=\"100%\" border=\"1\">";

    str += body.arg( i18nc( "Name of the plugin", "Name" ),                             s->name() );
    str += body.arg( i18nc( "Library name", "Library" ),                                s->library() );
    str += body.arg( i18nc( "Plugin authors", "Authors" ),                              s->property( "X-KDE-akregator-authors" ).toStringList().join( "\n" ) );
    str += body.arg( i18nc( "Plugin authors' emaila addresses", "Email" ),              s->property( "X-KDE-akregator-email" ).toStringList().join( "\n" ) );
    str += body.arg( i18nc( "Plugin version", "Version" ),                              s->property( "X-KDE-akregator-version" ).toString() );
    str += body.arg( i18nc( "Framework version plugin requires", "Framework Version" ), s->property( "X-KDE-akregator-framework-version" ).toString() );

    str += "</table></body></html>";

    KMessageBox::information( 0, str, i18n( "Plugin Information" ) );
}


void
PluginManager::dump( const KService::Ptr service )
{
    kDebug()
      << "PluginManager Service Info:" << endl
      << "---------------------------" << endl
      << "name                          : " << service->name() << endl
      << "library                       : " << service->library() << endl
      << "desktopEntryPath              : " << service->entryPath() << endl
      << "X-KDE-akregator-plugintype       : " << service->property( "X-KDE-akregator-plugintype" ).toString() << endl
      << "X-KDE-akregator-name             : " << service->property( "X-KDE-akregator-name" ).toString() << endl
      << "X-KDE-akregator-authors          : " << service->property( "X-KDE-akregator-authors" ).toStringList() << endl
      << "X-KDE-akregator-rank             : " << service->property( "X-KDE-akregator-rank" ).toString() << endl
      << "X-KDE-akregator-version          : " << service->property( "X-KDE-akregator-version" ).toString() << endl
      << "X-KDE-akregator-framework-version: " << service->property( "X-KDE-akregator-framework-version" ).toString()
      << endl;

}


/////////////////////////////////////////////////////////////////////////////////////
// PRIVATE INTERFACE
/////////////////////////////////////////////////////////////////////////////////////

vector<PluginManager::StoreItem>::iterator
PluginManager::lookupPlugin( const Plugin* plugin )
{
    vector<StoreItem>::iterator iter;

    //search plugin pointer in store
    vector<StoreItem>::const_iterator end;
    for ( iter = m_store.begin(); iter != end; ++iter ) {
        if ( (*iter).plugin == plugin )
            break;
    }

    return iter;
}

} // namespace Akregator
