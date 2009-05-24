/*
    This file is part of Akregator.

    Copyright (C) 2004 Mark Kretschmann <kretschmann@kde.org>

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

#ifndef AKREGATOR_PLUGIN_H
#define AKREGATOR_PLUGIN_H

#include "akregator_export.h"

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVariant>

class KXMLGUIClient;

#define AKREGATOR_PLUGIN_INTERFACE_VERSION 4

namespace Akregator {

class AKREGATORINTERFACES_EXPORT Plugin : public QObject
{
    Q_OBJECT
    public:
        explicit Plugin( QObject* parent=0, const QVariantList& opts=QVariantList() );

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

        virtual void insertGuiClients( KXMLGUIClient* parent );
        virtual void removeGuiClients( KXMLGUIClient* parent );

    protected:
        virtual void doInitialize() = 0;

    private:
        QHash<QString, QString> m_properties;
};

} //namespace Akregator

#endif // AKREGATOR_PLUGIN_H


