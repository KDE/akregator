/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxplugin.h"
#include "miniflux_debug.h"
#include "minifluxaccount.h"
#include "minifluxaccountdialog.h"

#include "kernel.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(MinifluxPlugin, "miniflux_plugin.json")

MinifluxPlugin::MinifluxPlugin(QObject *parent, const QVariantList &args)
    : AccountPlugin(parent)
{
    Q_UNUSED(args)
}

MinifluxPlugin::~MinifluxPlugin() = default;

void MinifluxPlugin::initialize(FeedList *feedList)
{
    m_feedList = feedList;
    // The part (and with it the feed list) can be destroyed and recreated while
    // the plugin survives in the Kernel singleton (e.g. inside Kontact); drop
    // accounts belonging to the previous feed list before reloading.
    for (const QPointer<MinifluxAccount> &account : std::as_const(m_accounts)) {
        delete account;
    }
    m_accounts.clear();
    loadSavedAccounts();
}

QString MinifluxPlugin::name() const
{
    return i18n("Miniflux");
}

void MinifluxPlugin::addAccount()
{
    if (!m_feedList) {
        qCWarning(MINIFLUX_LOG) << "Cannot add a Miniflux account before the feed list is loaded";
        return;
    }
    auto *dialog = new MinifluxAccountDialog(nullptr);
    if (dialog->exec() == QDialog::Accepted) {
        const QString accountName = dialog->accountName();
        const QUrl serverUrl = dialog->serverUrl();
        const QString apiToken = dialog->apiToken();

        // Save to config
        KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("MinifluxAccounts"));
        QStringList accounts = group.readEntry("accounts", QStringList());
        accounts.append(accountName);
        group.writeEntry("accounts", accounts);
        KConfigGroup accountGroup = KSharedConfig::openConfig()->group(QStringLiteral("MinifluxAccount-") + accountName);
        accountGroup.writeEntry("serverUrl", serverUrl.toString());
        accountGroup.writeEntry("apiToken", apiToken); // TODO: store the API token in KWallet
        KSharedConfig::openConfig()->sync();

        auto *account = new MinifluxAccount(accountName, serverUrl, apiToken, m_feedList, Kernel::self()->storage(), this);
        connect(account, &MinifluxAccount::accountDeleted, this, &MinifluxPlugin::onAccountDeleted);
        m_accounts.append(account);
        account->initialize();
    }
    dialog->deleteLater();
}

void MinifluxPlugin::onAccountDeleted(MinifluxAccount *account)
{
    m_accounts.removeOne(account);
    account->deleteLater();
}

void MinifluxPlugin::loadSavedAccounts()
{
    const KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("MinifluxAccounts"));
    const QStringList accounts = group.readEntry("accounts", QStringList());
    qCDebug(MINIFLUX_LOG) << "Loading saved Miniflux accounts:" << accounts;
    for (const QString &accountName : accounts) {
        const KConfigGroup accountGroup = KSharedConfig::openConfig()->group(QStringLiteral("MinifluxAccount-") + accountName);
        const QUrl serverUrl(accountGroup.readEntry("serverUrl"));
        const QString apiToken = accountGroup.readEntry("apiToken");
        if (!serverUrl.isValid() || apiToken.isEmpty()) {
            continue;
        }
        auto *account = new MinifluxAccount(accountName, serverUrl, apiToken, m_feedList, Kernel::self()->storage(), this);
        connect(account, &MinifluxAccount::accountDeleted, this, &MinifluxPlugin::onAccountDeleted);
        m_accounts.append(account);
        account->initialize();
    }
}

#include "minifluxplugin.moc"
