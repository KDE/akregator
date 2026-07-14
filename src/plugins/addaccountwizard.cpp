/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "addaccountwizard.h"
#include "accounteditwidget.h"
#include "accountplugin.h"

#include <KLocalizedString>
#include <QListWidget>
#include <QVBoxLayout>

using namespace Akregator;

AddAccountWizard::AddAccountWizard(const QList<AccountPlugin *> &plugins, QWidget *parent)
    : KAssistantDialog(parent)
    , m_plugins(plugins)
{
    setWindowTitle(i18nc("@title:window", "Add Online Account"));

    m_typeList = new QListWidget(this);
    for (AccountPlugin *plugin : plugins) {
        m_typeList->addItem(plugin->name());
    }
    m_typePage = addPage(m_typeList, i18nc("@title:tab, wizard page for selecting the account type", "Account Type"));

    m_settingsContainer = new QWidget(this);
    auto containerLayout = new QVBoxLayout(m_settingsContainer);
    containerLayout->setContentsMargins({});
    m_settingsPage = addPage(m_settingsContainer, i18nc("@title:tab, wizard page with the account settings", "Account Settings"));

    setValid(m_typePage, false);
    setValid(m_settingsPage, false);
    connect(m_typeList, &QListWidget::itemSelectionChanged, this, [this]() {
        setValid(m_typePage, m_typeList->currentItem() != nullptr);
    });
    connect(m_typeList, &QListWidget::itemDoubleClicked, this, &AddAccountWizard::next);

    if (!plugins.isEmpty()) {
        m_typeList->setCurrentRow(0);
    }
}

AddAccountWizard::~AddAccountWizard() = default;

void AddAccountWizard::next()
{
    if (currentPage() == m_typePage) {
        const int row = m_typeList->currentRow();
        if (row < 0 || row >= m_plugins.size()) {
            return;
        }
        AccountPlugin *plugin = m_plugins.at(row);
        if (plugin != m_currentPlugin) {
            delete m_editWidget;
            m_editWidget = plugin->createAccountEditWidget(m_settingsContainer);
            m_currentPlugin = plugin;
            if (!m_editWidget) {
                return;
            }
            m_settingsContainer->layout()->addWidget(m_editWidget);
            setValid(m_settingsPage, m_editWidget->isValid());
            connect(m_editWidget, &AccountEditWidget::validityChanged, this, [this]() {
                setValid(m_settingsPage, m_editWidget->isValid());
            });
        }
    }
    KAssistantDialog::next();
}

void AddAccountWizard::accept()
{
    if (m_editWidget && m_editWidget->isValid() && m_editWidget->apply()) {
        KAssistantDialog::accept();
    }
}

#include "moc_addaccountwizard.cpp"
