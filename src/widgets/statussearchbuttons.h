/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include "akregatorpart_private_export.h"
#include "statussearchline.h"
#include <QWidget>
class QButtonGroup;
namespace Akregator
{
class AKREGATORPART_TESTS_EXPORT StatusSearchButtons : public QWidget
{
    Q_OBJECT
public:
    explicit StatusSearchButtons(QWidget *parent = nullptr);
    ~StatusSearchButtons() override;

    void setStatus(StatusSearchLine::Status status);

    [[nodiscard]] StatusSearchLine::Status status() const;

Q_SIGNALS:
    void statusChanged(Akregator::StatusSearchLine::Status status);

private:
    QButtonGroup *const mButtonGroup;
};
}
