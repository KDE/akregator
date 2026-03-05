/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxplugin.h"
#include "minifluxaccount.h"
#include "minifluxaccountdialog.h"

#include "kernel.h"
#include "storage/storage.h"

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
    loadSavedAccounts();
}

QString MinifluxPlugin::name() const
{
    return i18n("Miniflux");
}

void MinifluxPlugin::addAccount()
{
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
        accountGroup.writeEntry("apiToken", apiToken); // TODO: use KWallet if HAVE_KWALLET
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
