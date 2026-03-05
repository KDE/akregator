/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxaccountdialog.h"
#include "minifluxclient.h"
#include "ui_minifluxaccountdialog.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QDialogButtonBox>
#include <QPointer>
#include <QPushButton>

using namespace Akregator;

MinifluxAccountDialog::MinifluxAccountDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MinifluxAccountDialog)
{
    ui->setupUi(this);
    setWindowTitle(i18nc("@title:window", "Add Miniflux Account"));
    connect(ui->testButton, &QPushButton::clicked, this, &MinifluxAccountDialog::slotTestConnection);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

MinifluxAccountDialog::~MinifluxAccountDialog()
{
    delete ui;
}

QString MinifluxAccountDialog::accountName() const
{
    return ui->accountNameEdit->text().trimmed();
}

QUrl MinifluxAccountDialog::serverUrl() const
{
    return QUrl(ui->serverUrlEdit->text().trimmed());
}

QString MinifluxAccountDialog::apiToken() const
{
    return ui->apiTokenEdit->text().trimmed();
}

void MinifluxAccountDialog::slotTestConnection()
{
    const QUrl url = serverUrl();
    const QString token = apiToken();
    if (!url.isValid() || token.isEmpty()) {
        KMessageBox::error(this, i18n("Please enter a valid server URL and API token."), i18nc("@title:window", "Invalid Input"));
        return;
    }
    ui->testButton->setEnabled(false);
    auto *client = new MinifluxClient(url, token, this);
    connect(client, &MinifluxClient::credentialsVerified, this, [this, client = QPointer<MinifluxClient>(client)](bool ok, const QString &error) {
        ui->testButton->setEnabled(true);
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

#include "moc_minifluxaccountdialog.cpp"
