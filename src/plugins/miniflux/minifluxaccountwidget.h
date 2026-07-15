/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "accounteditwidget.h"
#include <QUrl>

class QLineEdit;
class QPushButton;

namespace Akregator
{
class MinifluxPlugin;

/**
 * Configuration form for a Miniflux account (name, server URL, API token,
 * test-connection button). Used by the "Add Online Account" wizard and by
 * MinifluxAccountDialog for editing an existing account.
 */
class MinifluxAccountWidget : public AccountEditWidget
{
    Q_OBJECT
public:
    /** @param plugin used by apply() to create the account; may be nullptr
     *  when the widget is only used for editing. */
    explicit MinifluxAccountWidget(MinifluxPlugin *plugin, QWidget *parent = nullptr);
    ~MinifluxAccountWidget() override;

    [[nodiscard]] bool isValid() const override;
    bool apply() override;

    [[nodiscard]] QString accountName() const;
    [[nodiscard]] QUrl serverUrl() const;
    [[nodiscard]] QString apiToken() const;

    void setAccountName(const QString &accountName);
    void setAccountNameReadOnly(bool readOnly);
    void setServerUrl(const QUrl &serverUrl);
    void setApiToken(const QString &apiToken);

private Q_SLOTS:
    void slotTestConnection();

private:
    MinifluxPlugin *const m_plugin;
    QLineEdit *const m_accountNameEdit;
    QLineEdit *const m_serverUrlEdit;
    QLineEdit *const m_apiTokenEdit;
    QPushButton *const m_testButton;
};

} // namespace Akregator
