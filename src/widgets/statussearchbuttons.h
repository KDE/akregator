/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWidget>
class QButtonGroup;
namespace Akregator
{
class StatusSearchButtons : public QWidget
{
    Q_OBJECT
public:
    explicit StatusSearchButtons(QWidget *parent = nullptr);
    ~StatusSearchButtons() override;

private:
    QButtonGroup *const mButtonGroup;
};
}
