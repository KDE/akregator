/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "crashwidget/crashwidget.h"
#include <QStackedWidget>
namespace Akregator
{
class MainWidget;
class AkregatorCentralWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit AkregatorCentralWidget(QWidget *parent = nullptr);
    ~AkregatorCentralWidget();

    void needToRestoreCrashedSession();

    void setMainWidget(Akregator::MainWidget *mainWidget);
    Q_REQUIRED_RESULT bool previousSessionCrashed() const;

Q_SIGNALS:
    void restoreSession(Akregator::CrashWidget::CrashAction type);

private:
    void slotRestoreSession(Akregator::CrashWidget::CrashAction type);

    Akregator::CrashWidget *mCrashWidget = nullptr;
    Akregator::MainWidget *mMainWidget = nullptr;
};
}
