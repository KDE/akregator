/*
   SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "statussearchbuttonstest.h"
#include "widgets/statussearchbuttons.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QTest>

using namespace Qt::Literals::StringLiterals;
QTEST_MAIN(StatusSearchButtonsTest)
StatusSearchButtonsTest::StatusSearchButtonsTest(QObject *parent)
    : QObject{parent}
{
}

void StatusSearchButtonsTest::shouldHaveDefaultValues()
{
    Akregator::StatusSearchButtons w;

    auto mButtonGroup = w.findChild<QButtonGroup *>(u"mButtonGroup"_s);
    QVERIFY(mButtonGroup);
    QVERIFY(mButtonGroup->exclusive());

    auto mainLayout = w.findChild<QHBoxLayout *>(u"mainLayout"_s);
    QVERIFY(mainLayout);
}

#include "moc_statussearchbuttonstest.cpp"
