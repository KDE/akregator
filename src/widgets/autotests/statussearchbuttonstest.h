/*
   SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class StatusSearchButtonsTest : public QObject
{
    Q_OBJECT
public:
    explicit StatusSearchButtonsTest(QObject *parent = nullptr);
    ~StatusSearchButtonsTest() override = default;
private Q_SLOTS:
    void shouldHaveDefaultValues();
};
