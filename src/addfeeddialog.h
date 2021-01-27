/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_ADDFEEDDIALOG_H
#define AKREGATOR_ADDFEEDDIALOG_H

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
    ~AddFeedWidget();
};

class AddFeedDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddFeedDialog(QWidget *parent = nullptr, const QString &name = QString());
    ~AddFeedDialog() override;

    Q_REQUIRED_RESULT QSize sizeHint() const override;

    void setUrl(const QString &t);
    Feed *feed() const;

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

#endif // AKREGATOR_ADDFEEDDIALOG_H
