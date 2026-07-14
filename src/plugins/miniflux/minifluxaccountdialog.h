/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <QDialog>
#include <QUrl>

namespace Ui
{
class MinifluxAccountDialog;
}

namespace Akregator
{
class MinifluxClient;

class MinifluxAccountDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MinifluxAccountDialog(QWidget *parent = nullptr);
    ~MinifluxAccountDialog() override;

    /** Prefill the fields for editing an existing account. The account name
     *  becomes read-only, as it is used as the config and wallet key. */
    void setEditMode(const QString &accountName, const QUrl &serverUrl, const QString &apiToken);

    [[nodiscard]] QString accountName() const;
    [[nodiscard]] QUrl serverUrl() const;
    [[nodiscard]] QString apiToken() const;

private Q_SLOTS:
    void slotTestConnection();

private:
    Ui::MinifluxAccountDialog *const ui;
};

} // namespace Akregator
