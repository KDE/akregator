/*
   SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QLineEdit>
namespace Akregator
{
class StatusSearchLine : public QLineEdit
{
    Q_OBJECT
public:
    explicit StatusSearchLine(QWidget *parent = nullptr);
    ~StatusSearchLine() override;

Q_SIGNALS:
    void forceLostFocus();

protected:
    void keyPressEvent(QKeyEvent *e) override;
};
}
