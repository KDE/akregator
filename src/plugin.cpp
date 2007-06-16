// Author: Mark Kretschmann (C) Copyright 2004
// Copyright: See COPYING file that comes with this distribution

#include "plugin.h"


namespace Akregator {


Plugin::Plugin()
{}


Plugin::~Plugin()
{}


void
Plugin::addPluginProperty( const QString& key, const QString& value )
{
    m_properties[key.toLower()] = value;
}


void Plugin::initialize()
{
    doInitialize();
}

QString
Plugin::pluginProperty( const QString& key ) const
{
    if ( m_properties.find( key.toLower() ) == m_properties.end() )
        return "false";

    return m_properties[key.toLower()];
}


bool
Plugin::hasPluginProperty( const QString& key ) const
{
    return m_properties.find( key.toLower() ) != m_properties.end();
}

}
