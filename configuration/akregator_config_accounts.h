/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <KCModule>

class QPushButton;
class QTreeWidget;

namespace Akregator
{
class AccountPlugin;

/**
 * Config module listing the online accounts (e.g. Miniflux) provided by
 * account plugins, with buttons to add, modify and remove accounts.
 * All operations apply immediately through the running plugin instances.
 */
class KCMAkregatorAccountsConfig : public KCModule
{
    Q_OBJECT
public:
    explicit KCMAkregatorAccountsConfig(QObject *parent, const KPluginMetaData &data);

    void load() override;

private:
    void slotAddAccount();
    void slotModifyAccount();
    void slotRemoveAccount();
    void slotSelectionChanged();
    void fillAccountsList();
    [[nodiscard]] AccountPlugin *selectedPlugin() const;

    QTreeWidget *m_accountsList = nullptr;
    QPushButton *m_addButton = nullptr;
    QPushButton *m_modifyButton = nullptr;
    QPushButton *m_removeButton = nullptr;
};

} // namespace Akregator
