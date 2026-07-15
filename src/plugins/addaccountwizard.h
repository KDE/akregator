/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <KAssistantDialog>
#include <QList>

class QListWidget;

namespace Akregator
{
class AccountEditWidget;
class AccountPlugin;

/**
 * Wizard for adding an online account: the first page selects the account
 * type (one entry per loaded AccountPlugin), the second page embeds the
 * configuration form provided by the selected plugin.
 */
class AddAccountWizard : public KAssistantDialog
{
    Q_OBJECT
public:
    explicit AddAccountWizard(const QList<AccountPlugin *> &plugins, QWidget *parent = nullptr);
    ~AddAccountWizard() override;

protected:
    void next() override;
    void accept() override;

private:
    const QList<AccountPlugin *> m_plugins;
    AccountPlugin *m_currentPlugin = nullptr;
    AccountEditWidget *m_editWidget = nullptr;
    QListWidget *const m_typeList;
    QWidget *const m_settingsContainer;
    KPageWidgetItem *m_typePage = nullptr;
    KPageWidgetItem *m_settingsPage = nullptr;
};

} // namespace Akregator
