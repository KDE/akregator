/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Mark Kretschmann <kretschmann@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "plugin.h"

namespace Akregator {
Plugin::Plugin(QObject *parent, const QVariantList &) : QObject(parent)
{
}

Plugin::~Plugin()
{
}

void
Plugin::addPluginProperty(const QString &key, const QString &value)
{
    m_properties[key.toLower()] = value;
}

void Plugin::initialize()
{
    doInitialize();
}

QString
Plugin::pluginProperty(const QString &key) const
{
    if (m_properties.find(key.toLower()) == m_properties.end()) {
        return QStringLiteral("false");
    }

    return m_properties[key.toLower()];
}

bool
Plugin::hasPluginProperty(const QString &key) const
{
    return m_properties.find(key.toLower()) != m_properties.end();
}

void Plugin::insertGuiClients(KXMLGUIClient *)
{
}

void Plugin::removeGuiClients(KXMLGUIClient *)
{
}
}
