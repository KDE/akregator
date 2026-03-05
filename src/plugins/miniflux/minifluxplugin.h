/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "accountplugin.h"

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
    void addAccount() override;
    [[nodiscard]] QString name() const override;

private Q_SLOTS:
    void onAccountDeleted(MinifluxAccount *account);

private:
    void loadSavedAccounts();

    FeedList *m_feedList = nullptr;
    QList<MinifluxAccount *> m_accounts;
};

} // namespace Akregator
