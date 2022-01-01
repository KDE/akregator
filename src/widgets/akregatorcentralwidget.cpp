/*
   SPDX-FileCopyrightText: 2016-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregatorcentralwidget.h"
#include "mainwidget.h"

#include <KConfig>
#include <KConfigGroup>

#include <QStandardPaths>

using namespace Akregator;

AkregatorCentralWidget::AkregatorCentralWidget(QWidget *parent)
    : QStackedWidget(parent)
{
    mCrashWidget = new CrashWidget(this);
    connect(mCrashWidget, &CrashWidget::restoreSession, this, &AkregatorCentralWidget::slotRestoreSession);
    addWidget(mCrashWidget);
}

Akregator::AkregatorCentralWidget::~AkregatorCentralWidget() = default;

bool AkregatorCentralWidget::previousSessionCrashed() const
{
    KConfig config(QStringLiteral("crashed"), KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
    KConfigGroup configGroup(&config, "Part");

    if (!configGroup.readEntry("crashed", false)) {
        return false;
    }
    return true;
}

void AkregatorCentralWidget::needToRestoreCrashedSession()
{
    setCurrentWidget(mCrashWidget);
}

void AkregatorCentralWidget::slotRestoreSession(Akregator::CrashWidget::CrashAction type)
{
    setCurrentWidget(mMainWidget);
    Q_EMIT restoreSession(type);
}

void AkregatorCentralWidget::setMainWidget(Akregator::MainWidget *mainWidget)
{
    if (!mMainWidget) {
        mMainWidget = mainWidget;
        addWidget(mMainWidget);
        setCurrentWidget(mMainWidget);
    }
}
