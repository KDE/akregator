/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxaccountwidget.h"
#include "minifluxclient.h"
#include "minifluxplugin.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Akregator;

MinifluxAccountWidget::MinifluxAccountWidget(MinifluxPlugin *plugin, QWidget *parent)
    : AccountEditWidget(parent)
    , m_plugin(plugin)
{
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins({});

    auto formLayout = new QFormLayout;
    m_accountNameEdit = new QLineEdit(this);
    m_accountNameEdit->setPlaceholderText(i18n("My Miniflux"));
    formLayout->addRow(i18n("Account Name:"), m_accountNameEdit);
    m_serverUrlEdit = new QLineEdit(this);
    m_serverUrlEdit->setPlaceholderText(QStringLiteral("https://miniflux.example.com"));
    formLayout->addRow(i18n("Server URL:"), m_serverUrlEdit);
    m_apiTokenEdit = new QLineEdit(this);
    m_apiTokenEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow(i18n("API Token:"), m_apiTokenEdit);
    mainLayout->addLayout(formLayout);

    m_testButton = new QPushButton(i18nc("@action:button", "Test Connection"), this);
    mainLayout->addWidget(m_testButton);
    mainLayout->addStretch();

    connect(m_testButton, &QPushButton::clicked, this, &MinifluxAccountWidget::slotTestConnection);
    connect(m_accountNameEdit, &QLineEdit::textChanged, this, &AccountEditWidget::validityChanged);
    connect(m_serverUrlEdit, &QLineEdit::textChanged, this, &AccountEditWidget::validityChanged);
    connect(m_apiTokenEdit, &QLineEdit::textChanged, this, &AccountEditWidget::validityChanged);
}

MinifluxAccountWidget::~MinifluxAccountWidget() = default;

bool MinifluxAccountWidget::isValid() const
{
    return !accountName().isEmpty() && serverUrl().isValid() && !serverUrl().scheme().isEmpty() && !apiToken().isEmpty();
}

bool MinifluxAccountWidget::apply()
{
    return m_plugin && m_plugin->createAccountFromWizard(accountName(), serverUrl(), apiToken());
}

QString MinifluxAccountWidget::accountName() const
{
    return m_accountNameEdit->text().trimmed();
}

QUrl MinifluxAccountWidget::serverUrl() const
{
    return QUrl(m_serverUrlEdit->text().trimmed());
}

QString MinifluxAccountWidget::apiToken() const
{
    return m_apiTokenEdit->text().trimmed();
}

void MinifluxAccountWidget::setAccountName(const QString &accountName)
{
    m_accountNameEdit->setText(accountName);
}

void MinifluxAccountWidget::setAccountNameReadOnly(bool readOnly)
{
    m_accountNameEdit->setReadOnly(readOnly);
}

void MinifluxAccountWidget::setServerUrl(const QUrl &serverUrl)
{
    m_serverUrlEdit->setText(serverUrl.toString());
}

void MinifluxAccountWidget::setApiToken(const QString &apiToken)
{
    m_apiTokenEdit->setText(apiToken);
}

void MinifluxAccountWidget::slotTestConnection()
{
    const QUrl url = serverUrl();
    const QString token = apiToken();
    if (!url.isValid() || token.isEmpty()) {
        KMessageBox::error(this, i18n("Please enter a valid server URL and API token."), i18nc("@title:window", "Invalid Input"));
        return;
    }
    m_testButton->setEnabled(false);
    auto *client = new MinifluxClient(url, token, this);
    connect(client, &MinifluxClient::credentialsVerified, this, [this, client = QPointer<MinifluxClient>(client)](bool ok, const QString &error) {
        m_testButton->setEnabled(true);
        if (client) {
            client->deleteLater();
        }
        if (ok) {
            KMessageBox::information(this, i18n("Successfully connected to the Miniflux server."), i18nc("@title:window", "Connection Successful"));
        } else {
            KMessageBox::error(this, i18n("Failed to connect: %1", error), i18nc("@title:window", "Connection Failed"));
        }
    });
    client->verifyCredentials();
}

#include "moc_minifluxaccountwidget.cpp"
