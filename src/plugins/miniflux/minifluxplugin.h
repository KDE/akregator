/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "accountplugin.h"
#include <QPointer>
#include <QUrl>

namespace Akregator
{
class FeedList;
class MinifluxAccount;

class MinifluxPlugin : public AccountPlugin
{
    Q_OBJECT
public:
    explicit MinifluxPlugin(QObject *parent, const QVariantList &args);
    ~MinifluxPlugin() override;

    void initialize(FeedList *feedList) override;
    [[nodiscard]] AccountEditWidget *createAccountEditWidget(QWidget *parent) override;
    [[nodiscard]] QStringList accountNames() const override;
    void configureAccount(const QString &accountName) override;
    void removeAccount(const QString &accountName) override;
    [[nodiscard]] QString name() const override;

    /** Called by MinifluxAccountWidget::apply(): persists and creates the new
     *  account. @return false if the input is rejected (e.g. duplicate name). */
    bool createAccountFromWizard(const QString &accountName, const QUrl &serverUrl, const QString &apiToken);

private Q_SLOTS:
    void onAccountDeleted(MinifluxAccount *account);
    void onFeedListDestroyed();

private:
    void loadSavedAccounts();
    void createAccount(const QString &accountName, const QUrl &serverUrl, const QString &apiToken);
    void editAccount(MinifluxAccount *account);
    [[nodiscard]] MinifluxAccount *findAccount(const QString &accountName) const;
    void storeApiToken(const QString &accountName, const QString &apiToken);
    void deleteApiToken(const QString &accountName);

    FeedList *m_feedList = nullptr;
    QList<QPointer<MinifluxAccount>> m_accounts;
};

} // namespace Akregator
