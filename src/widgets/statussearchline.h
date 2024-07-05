/*
   SPDX-FileCopyrightText: 2016-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KLineEdit>
namespace Akregator
{
class StatusSearchLine : public KLineEdit
{
    Q_OBJECT
public:
    enum Status {
        AllArticles = 0,
        NewArticles,
        UnreadArticles,
        ReadArticles,
        ImportantArticles,
    };

    explicit StatusSearchLine(QWidget *parent = nullptr);
    ~StatusSearchLine() override;

Q_SIGNALS:
    void forceLostFocus();

protected:
    void keyPressEvent(QKeyEvent *e) override;
};
}
