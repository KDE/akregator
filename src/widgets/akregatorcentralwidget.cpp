/*
   Copyright (C) 2016-2020 Laurent Montel <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

Akregator::AkregatorCentralWidget::~AkregatorCentralWidget()
{
}

bool AkregatorCentralWidget::previousSessionCrashed() const
{
    KConfig config(QStringLiteral("crashed"), KConfig::SimpleConfig,
                   QStandardPaths::AppDataLocation);
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
