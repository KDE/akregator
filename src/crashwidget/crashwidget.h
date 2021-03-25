/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWidget>
namespace Akregator
{
class CrashWidget : public QWidget
{
    Q_OBJECT
public:
    enum CrashAction { RestoreSession = 0, NotRestoreSession, AskMeLater };

    explicit CrashWidget(QWidget *parent = nullptr);
    ~CrashWidget();

Q_SIGNALS:
    void restoreSession(Akregator::CrashWidget::CrashAction type);

private Q_SLOTS:
    void slotDontRestoreSession();
    void slotRestoreSession();
    void slotAskMeLater();
};
}

