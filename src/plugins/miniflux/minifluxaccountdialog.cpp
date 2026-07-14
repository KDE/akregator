/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "minifluxaccountdialog.h"
#include "minifluxaccountwidget.h"

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Akregator;

MinifluxAccountDialog::MinifluxAccountDialog(QWidget *parent)
    : QDialog(parent)
    , m_widget(new MinifluxAccountWidget(nullptr, this))
{
    setWindowTitle(i18nc("@title:window", "Miniflux Account Settings"));
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_widget);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setEnabled(m_widget->isValid());
    connect(m_widget, &AccountEditWidget::validityChanged, this, [this, okButton]() {
        okButton->setEnabled(m_widget->isValid());
    });
}

MinifluxAccountDialog::~MinifluxAccountDialog() = default;

void MinifluxAccountDialog::setEditMode(const QString &accountName, const QUrl &serverUrl, const QString &apiToken)
{
    m_widget->setAccountName(accountName);
    m_widget->setAccountNameReadOnly(true);
    m_widget->setServerUrl(serverUrl);
    m_widget->setApiToken(apiToken);
}

QString MinifluxAccountDialog::accountName() const
{
    return m_widget->accountName();
}

QUrl MinifluxAccountDialog::serverUrl() const
{
    return m_widget->serverUrl();
}

QString MinifluxAccountDialog::apiToken() const
{
    return m_widget->apiToken();
}

#include "moc_minifluxaccountdialog.cpp"
