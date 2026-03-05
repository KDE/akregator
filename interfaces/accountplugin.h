/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "akregatorinterfaces_export.h"
#include <KPluginFactory>
#include <QObject>

namespace Akregator
{
class Folder;
class FeedList;

/**
 * Abstract interface for account plugins (e.g. Miniflux, Fever, etc.)
 * Plugins implementing this interface are discovered and loaded by PluginManager.
 */
class AKREGATORINTERFACES_EXPORT AccountPlugin : public QObject
{
    Q_OBJECT
public:
    explicit AccountPlugin(QObject *parent = nullptr);
    ~AccountPlugin() override;

    /** Called by PluginManager after the plugin is loaded. feedList is the global feed list. */
    virtual void initialize(FeedList *feedList) = 0;

    /** Called when the user triggers "Add Account" for this plugin type. */
    virtual void addAccount() = 0;

    /** Display name for this plugin type (e.g. "Miniflux") */
    [[nodiscard]] virtual QString name() const = 0;
};

} // namespace Akregator
