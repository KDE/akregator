/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "statussearchbuttonstest.h"
#include "widgets/statussearchbuttons.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QTest>
QTEST_MAIN(StatusSearchButtonsTest)
StatusSearchButtonsTest::StatusSearchButtonsTest(QObject *parent)
    : QObject{parent}
{
}

void StatusSearchButtonsTest::shouldHaveDefaultValues()
{
    Akregator::StatusSearchButtons w;

    auto mButtonGroup = w.findChild<QButtonGroup *>(QStringLiteral("mButtonGroup"));
    QVERIFY(mButtonGroup);
    QVERIFY(mButtonGroup->exclusive());

    auto mainLayout = w.findChild<QHBoxLayout *>(QStringLiteral("mainLayout"));
    QVERIFY(mainLayout);
    QCOMPARE(mainLayout->contentsMargins(), QMargins{});
    QCOMPARE(mainLayout->spacing(), 0);
}
