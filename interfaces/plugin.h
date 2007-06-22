// Author: Mark Kretschmann (C) Copyright 2004
// Copyright: See COPYING file that comes with this distribution

#ifndef AKREGATOR_PLUGIN_H
#define AKREGATOR_PLUGIN_H

#include "akregator_export.h"

#define AKREGATOR_EXPORT_PLUGIN( classname ) \
    extern "C" { \
         KDE_EXPORT Akregator::Plugin* create_plugin() { return new classname; } \
    }

#include <QHash>
#include <QString>


namespace Akregator {

//    class PluginConfig;

class AKREGATOR_EXPORT Plugin
{
    public:
        virtual ~Plugin();

        void initialize();
        /**
            * TODO @param parent you must parent the widget to parent
            * @return the configure widget for your plugin, create it on the heap!
            */
            //TODO rename configureWidget( QWidget *parent )
        // virtual PluginConfig* configure() const { return 0; }

        void addPluginProperty( const QString& key, const QString& value );
        QString pluginProperty( const QString& key ) const;
        bool hasPluginProperty( const QString& key ) const;

    protected:
        Plugin();
        virtual void doInitialize() = 0;

    private:
        QHash<QString, QString> m_properties;
};

} //namespace Akregator

#endif // AKREGATOR_PLUGIN_H


