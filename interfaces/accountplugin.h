/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "akregatorinterfaces_export.h"
#include <KPluginFactory>
#include <QObject>
#include <QStringList>

namespace Akregator
{
class AccountEditWidget;
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

    /** Create the configuration form for a new account of this type. It is
     *  embedded into the "Add Online Account" wizard, which takes ownership. */
    [[nodiscard]] virtual AccountEditWidget *createAccountEditWidget(QWidget *parent) = 0;

    /** Names of the currently configured accounts of this plugin. */
    [[nodiscard]] virtual QStringList accountNames() const;

    /** Open the settings dialog for the given account.
     *  The default implementation does nothing. */
    virtual void configureAccount(const QString &accountName);

    /** Remove the given account: its folder in the feed tree, its configuration
     *  and its stored credentials. The default implementation does nothing. */
    virtual void removeAccount(const QString &accountName);

    /** Display name for this plugin type (e.g. "Miniflux") */
    [[nodiscard]] virtual QString name() const = 0;
};

} // namespace Akregator
