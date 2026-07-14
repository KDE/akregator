/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "akregator_config_accounts.h"
#include "accountplugin.h"
#include "addaccountwizard.h"
#include "kernel.h"
#include "pluginmanager.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KStandardGuiItem>

#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorAccountsConfig, "akregator_config_accounts.json")

namespace
{
constexpr int PluginNameRole = Qt::UserRole;
constexpr int AccountNameRole = Qt::UserRole + 1;
}

KCMAkregatorAccountsConfig::KCMAkregatorAccountsConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    auto mainLayout = new QHBoxLayout(widget());
    mainLayout->setContentsMargins({});

    m_accountsList = new QTreeWidget(widget());
    m_accountsList->setHeaderLabels({i18nc("@title:column", "Account"), i18nc("@title:column", "Type")});
    m_accountsList->setRootIsDecorated(false);
    m_accountsList->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(m_accountsList);

    auto buttonLayout = new QVBoxLayout;
    m_addButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-add")), i18nc("@action:button", "Add…"), widget());
    m_modifyButton = new QPushButton(QIcon::fromTheme(QStringLiteral("document-edit")), i18nc("@action:button", "Modify…"), widget());
    m_removeButton = new QPushButton(QIcon::fromTheme(QStringLiteral("list-remove")), i18nc("@action:button", "Remove"), widget());
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_modifyButton);
    buttonLayout->addWidget(m_removeButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    connect(m_addButton, &QPushButton::clicked, this, &KCMAkregatorAccountsConfig::slotAddAccount);
    connect(m_modifyButton, &QPushButton::clicked, this, &KCMAkregatorAccountsConfig::slotModifyAccount);
    connect(m_removeButton, &QPushButton::clicked, this, &KCMAkregatorAccountsConfig::slotRemoveAccount);
    connect(m_accountsList, &QTreeWidget::itemSelectionChanged, this, &KCMAkregatorAccountsConfig::slotSelectionChanged);
    connect(m_accountsList, &QTreeWidget::itemDoubleClicked, this, &KCMAkregatorAccountsConfig::slotModifyAccount);

    slotSelectionChanged();
}

void KCMAkregatorAccountsConfig::load()
{
    fillAccountsList();
    setNeedsSave(false);
}

void KCMAkregatorAccountsConfig::fillAccountsList()
{
    m_accountsList->clear();
    const auto plugins = Kernel::self()->pluginManager()->accountPlugins();
    for (AccountPlugin *plugin : plugins) {
        const QStringList names = plugin->accountNames();
        for (const QString &accountName : names) {
            auto item = new QTreeWidgetItem(m_accountsList);
            item->setText(0, accountName);
            item->setText(1, plugin->name());
            item->setData(0, PluginNameRole, plugin->name());
            item->setData(0, AccountNameRole, accountName);
        }
    }
    m_accountsList->resizeColumnToContents(0);
    slotSelectionChanged();
}

AccountPlugin *KCMAkregatorAccountsConfig::selectedPlugin() const
{
    const QTreeWidgetItem *item = m_accountsList->currentItem();
    if (!item) {
        return nullptr;
    }
    const QString pluginName = item->data(0, PluginNameRole).toString();
    const auto plugins = Kernel::self()->pluginManager()->accountPlugins();
    for (AccountPlugin *plugin : plugins) {
        if (plugin->name() == pluginName) {
            return plugin;
        }
    }
    return nullptr;
}

void KCMAkregatorAccountsConfig::slotAddAccount()
{
    const auto plugins = Kernel::self()->pluginManager()->accountPlugins();
    if (plugins.isEmpty()) {
        return;
    }
    AddAccountWizard wizard(plugins, widget());
    wizard.exec();
    fillAccountsList();
}

void KCMAkregatorAccountsConfig::slotModifyAccount()
{
    const QTreeWidgetItem *item = m_accountsList->currentItem();
    AccountPlugin *plugin = selectedPlugin();
    if (!item || !plugin) {
        return;
    }
    plugin->configureAccount(item->data(0, AccountNameRole).toString());
    fillAccountsList();
}

void KCMAkregatorAccountsConfig::slotRemoveAccount()
{
    const QTreeWidgetItem *item = m_accountsList->currentItem();
    AccountPlugin *plugin = selectedPlugin();
    if (!item || !plugin) {
        return;
    }
    const QString accountName = item->data(0, AccountNameRole).toString();
    const int answer = KMessageBox::warningContinueCancel(widget(),
                                                          i18n("Do you really want to remove the account \"%1\"?\n"
                                                               "This removes the account's folder from the feed tree and deletes its stored "
                                                               "credentials; the server is not affected.",
                                                               accountName),
                                                          i18nc("@title:window", "Remove Account"),
                                                          KStandardGuiItem::remove());
    if (answer != KMessageBox::Continue) {
        return;
    }
    plugin->removeAccount(accountName);
    fillAccountsList();
}

void KCMAkregatorAccountsConfig::slotSelectionChanged()
{
    const bool hasSelection = m_accountsList->currentItem() != nullptr;
    m_modifyButton->setEnabled(hasSelection);
    m_removeButton->setEnabled(hasSelection);
}

#include "akregator_config_accounts.moc"

#include "moc_akregator_config_accounts.cpp"
