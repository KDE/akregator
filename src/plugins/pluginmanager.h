/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <QList>
#include <QObject>

namespace Akregator
{
class AccountPlugin;
class FeedList;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager() override;

    /** Discovers and instantiates all account plugins. Call before GUI is built. */
    void instantiatePlugins();

    /** Initializes loaded plugins with the feed list. Call once the feed list is available. */
    void initializePlugins(FeedList *feedList);

    /** Returns all loaded account plugins. */
    [[nodiscard]] QList<AccountPlugin *> accountPlugins() const;

private:
    QList<AccountPlugin *> m_accountPlugins;
};

} // namespace Akregator
