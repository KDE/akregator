/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Mark Kretschmann <kretschmann@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregatorinterfaces_export.h"

#include <QHash>
#include <QObject>
#include <QString>
#include <QVariant>

class KXMLGUIClient;

#define AKREGATOR_PLUGIN_INTERFACE_VERSION 4

namespace Akregator
{
class AKREGATORINTERFACES_EXPORT Plugin : public QObject
{
    Q_OBJECT
public:
    explicit Plugin(QObject *parent = nullptr, const QVariantList &opts = QVariantList());

    ~Plugin() override;

    void initialize();

    void addPluginProperty(const QString &key, const QString &value);
    Q_REQUIRED_RESULT QString pluginProperty(const QString &key) const;
    bool hasPluginProperty(const QString &key) const;

    virtual void insertGuiClients(KXMLGUIClient *parent);
    virtual void removeGuiClients(KXMLGUIClient *parent);

protected:
    virtual void doInitialize() = 0;

private:
    QHash<QString, QString> m_properties;
};
} // namespace Akregator
