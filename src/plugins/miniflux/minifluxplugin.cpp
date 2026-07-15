/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxplugin.h"
#include "feedlist.h"
#include "miniflux_debug.h"
#include "minifluxaccount.h"
#include "minifluxaccountdialog.h"
#include "minifluxaccountwidget.h"

#include "kernel.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KSharedConfig>
#include <qt6keychain/keychain.h>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(MinifluxPlugin, "miniflux_plugin.json")

namespace
{
QString keychainServiceName()
{
    return QStringLiteral("akregator");
}

QString keychainKey(const QString &accountName)
{
    return QStringLiteral("miniflux-") + accountName;
}

KConfigGroup accountConfigGroup(const QString &accountName)
{
    return KSharedConfig::openConfig()->group(QStringLiteral("MinifluxAccount-") + accountName);
}
}

MinifluxPlugin::MinifluxPlugin(QObject *parent, [[maybe_unused]] const QVariantList &args)
    : AccountPlugin(parent)
{
}

MinifluxPlugin::~MinifluxPlugin() = default;

void MinifluxPlugin::initialize(FeedList *feedList)
{
    m_feedList = feedList;
    connect(feedList, &FeedList::signalDestroyed, this, &MinifluxPlugin::onFeedListDestroyed);
    // The part (and with it the feed list) can be destroyed and recreated while
    // the plugin survives in the Kernel singleton (e.g. inside Kontact); drop
    // accounts belonging to the previous feed list before reloading.
    for (const QPointer<MinifluxAccount> &account : std::as_const(m_accounts)) {
        delete account;
    }
    m_accounts.clear();
    loadSavedAccounts();
}

void MinifluxPlugin::onFeedListDestroyed()
{
    m_feedList = nullptr;
}

QString MinifluxPlugin::name() const
{
    return i18n("Miniflux");
}

AccountEditWidget *MinifluxPlugin::createAccountEditWidget(QWidget *parent)
{
    return new MinifluxAccountWidget(this, parent);
}

bool MinifluxPlugin::createAccountFromWizard(const QString &accountName, const QUrl &serverUrl, const QString &apiToken)
{
    if (!m_feedList) {
        qCWarning(MINIFLUX_LOG) << "Cannot add a Miniflux account before the feed list is loaded";
        return false;
    }
    KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("MinifluxAccounts"));
    QStringList accounts = group.readEntry("accounts", QStringList());
    if (accounts.contains(accountName)) {
        KMessageBox::error(nullptr, i18n("An account named \"%1\" already exists.", accountName), i18nc("@title:window", "Add Online Account"));
        return false;
    }

    // The server URL goes to the config file, the API token to the wallet.
    accounts.append(accountName);
    group.writeEntry("accounts", accounts);
    KConfigGroup accountGroup = accountConfigGroup(accountName);
    accountGroup.writeEntry("serverUrl", serverUrl.toString());
    KSharedConfig::openConfig()->sync();
    storeApiToken(accountName, apiToken);

    createAccount(accountName, serverUrl, apiToken);
    return true;
}

QStringList MinifluxPlugin::accountNames() const
{
    QStringList names;
    for (const QPointer<MinifluxAccount> &account : std::as_const(m_accounts)) {
        if (account) {
            names.append(account->accountName());
        }
    }
    return names;
}

MinifluxAccount *MinifluxPlugin::findAccount(const QString &accountName) const
{
    for (const QPointer<MinifluxAccount> &account : std::as_const(m_accounts)) {
        if (account && account->accountName() == accountName) {
            return account;
        }
    }
    return nullptr;
}

void MinifluxPlugin::configureAccount(const QString &accountName)
{
    if (MinifluxAccount *account = findAccount(accountName)) {
        editAccount(account);
    }
}

void MinifluxPlugin::removeAccount(const QString &accountName)
{
    if (MinifluxAccount *account = findAccount(accountName)) {
        // Removing the folder routes through the same path as deleting it from
        // the feed tree: config and wallet entries are cleaned up and
        // accountDeleted is emitted.
        account->removeFromTree();
    }
}

void MinifluxPlugin::editAccount(MinifluxAccount *account)
{
    const QString accountName = account->accountName();
    auto dialog = new MinifluxAccountDialog(nullptr);
    dialog->setEditMode(accountName, account->serverUrl(), account->apiToken());
    if (dialog->exec() == QDialog::Accepted) {
        const QUrl serverUrl = dialog->serverUrl();
        const QString apiToken = dialog->apiToken();

        KConfigGroup accountGroup = accountConfigGroup(accountName);
        accountGroup.writeEntry("serverUrl", serverUrl.toString());
        KSharedConfig::openConfig()->sync();
        storeApiToken(accountName, apiToken);

        account->setCredentials(serverUrl, apiToken);
    }
    dialog->deleteLater();
}

void MinifluxPlugin::onAccountDeleted(MinifluxAccount *account)
{
    deleteApiToken(account->accountName());
    m_accounts.removeOne(account);
    account->deleteLater();
}

void MinifluxPlugin::createAccount(const QString &accountName, const QUrl &serverUrl, const QString &apiToken)
{
    if (!m_feedList) {
        // The feed list went away while the wallet was being read.
        return;
    }
    auto account = new MinifluxAccount(accountName, serverUrl, apiToken, m_feedList, Kernel::self()->storage(), this);
    connect(account, &MinifluxAccount::accountDeleted, this, &MinifluxPlugin::onAccountDeleted);
    m_accounts.append(account);
    account->initialize();
}

void MinifluxPlugin::loadSavedAccounts()
{
    const KConfigGroup group = KSharedConfig::openConfig()->group(QStringLiteral("MinifluxAccounts"));
    const QStringList accounts = group.readEntry("accounts", QStringList());
    qCDebug(MINIFLUX_LOG) << "Loading saved Miniflux accounts:" << accounts;
    for (const QString &accountName : accounts) {
        const QUrl serverUrl(accountConfigGroup(accountName).readEntry("serverUrl"));
        if (!serverUrl.isValid()) {
            continue;
        }
        // The API token is read asynchronously from the wallet.
        auto job = new QKeychain::ReadPasswordJob(keychainServiceName(), this);
        job->setKey(keychainKey(accountName));
        connect(job, &QKeychain::Job::finished, this, [this, accountName, serverUrl](QKeychain::Job *baseJob) {
            QString apiToken = static_cast<QKeychain::ReadPasswordJob *>(baseJob)->textData();
            if (baseJob->error() != QKeychain::NoError || apiToken.isEmpty()) {
                // Fall back to the legacy plaintext config entry and migrate it to
                // the wallet; the plaintext copy is removed once the wallet write
                // has succeeded (see storeApiToken()).
                apiToken = accountConfigGroup(accountName).readEntry("apiToken");
                if (apiToken.isEmpty()) {
                    qCWarning(MINIFLUX_LOG) << "No API token found for Miniflux account" << accountName << ":" << baseJob->errorString();
                    return;
                }
                storeApiToken(accountName, apiToken);
            }
            createAccount(accountName, serverUrl, apiToken);
        });
        job->start();
    }
}

void MinifluxPlugin::storeApiToken(const QString &accountName, const QString &apiToken)
{
    auto job = new QKeychain::WritePasswordJob(keychainServiceName(), this);
    job->setKey(keychainKey(accountName));
    job->setTextData(apiToken);
    connect(job, &QKeychain::Job::finished, this, [accountName, apiToken](QKeychain::Job *baseJob) {
        KConfigGroup accountGroup = accountConfigGroup(accountName);
        if (baseJob->error() != QKeychain::NoError) {
            qCWarning(MINIFLUX_LOG) << "Failed to store the API token for Miniflux account" << accountName << "in the wallet:" << baseJob->errorString()
                                    << "- keeping it in the config file";
            accountGroup.writeEntry("apiToken", apiToken);
        } else if (accountGroup.hasKey("apiToken")) {
            // The token is safely in the wallet; drop the legacy plaintext copy.
            accountGroup.deleteEntry("apiToken");
        }
        KSharedConfig::openConfig()->sync();
    });
    job->start();
}

void MinifluxPlugin::deleteApiToken(const QString &accountName)
{
    auto job = new QKeychain::DeletePasswordJob(keychainServiceName(), this);
    job->setKey(keychainKey(accountName));
    job->start();
}

#include "minifluxplugin.moc"
