/*
   SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include "akregator_export.h"
#include <QWidget>
class QButtonGroup;
namespace Akregator
{
class AKREGATOR_EXPORT StatusSearchButtons : public QWidget
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
    explicit StatusSearchButtons(QWidget *parent = nullptr);
    ~StatusSearchButtons() override;

    void setStatus(StatusSearchButtons::Status status);

    [[nodiscard]] StatusSearchButtons::Status status() const;

Q_SIGNALS:
    void statusChanged(Akregator::StatusSearchButtons::Status status);

private:
    QButtonGroup *const mButtonGroup;
};
}
