// Author: Mark Kretschmann (C) Copyright 2004
// Copyright: See COPYING file that comes with this distribution

#ifndef AKREGATOR_PLUGIN_H
#define AKREGATOR_PLUGIN_H

#define AKREGATOR_EXPORT_PLUGIN( classname ) \
    extern "C" { \
         Akregator::Plugin* create_plugin() { return new classname; } \
    }

#include <qmap.h>
#include <qstring.h>


namespace Akregator
{
//    class PluginConfig;

    class Plugin
    {
        public:
            virtual ~Plugin();

            virtual bool init() = 0;
            /**
             * TODO @param parent you must parent the widget to parent
             * @return the configure widget for your plugin, create it on the heap!
             */
             //TODO rename configureWidget( QWidget *parent )
            // virtual PluginConfig* configure() const { return 0; }

            void addPluginProperty( const QString& key, const QString& value );
            QString pluginProperty( const QString& key );
            bool hasPluginProperty( const QString& key );

        protected:
            Plugin();

        private:
            QMap<QString, QString> m_properties;
    };

} //namespace Akregator


#endif /* AKREGATOR_PLUGIN_H */


