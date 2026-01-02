/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "ui_addfeedwidgetbase.h"

#include <QDialog>

#include <QWidget>
class QPushButton;
namespace Akregator
{
class Feed;

class AddFeedWidget : public QWidget, public Ui::AddFeedWidgetBase
{
    Q_OBJECT
public:
    explicit AddFeedWidget(QWidget *parent = nullptr);
    ~AddFeedWidget() override;
};

class AddFeedDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddFeedDialog(QWidget *parent = nullptr, const QString &name = QString());
    ~AddFeedDialog() override;

    [[nodiscard]] QSize sizeHint() const override;

    void setUrl(const QString &t);
    [[nodiscard]] Feed *feed() const;

public Q_SLOTS:
    void accept() override;

    void fetchCompleted(Akregator::Feed *);
    void fetchDiscovery(Akregator::Feed *);
    void fetchError(Akregator::Feed *);

private:
    void textChanged(const QString &);
    QString mFeedUrl;
    AddFeedWidget *widget = nullptr;
    Feed *m_feed = nullptr;
    QPushButton *mOkButton = nullptr;
};
} // namespace Akregator
