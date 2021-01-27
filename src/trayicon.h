/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_TRAYICON_H
#define AKREGATOR_TRAYICON_H

#include "akregator_export.h"

#include <KStatusNotifierItem>
namespace Akregator
{
class AKREGATOR_EXPORT TrayIcon : public KStatusNotifierItem
{
    Q_OBJECT
public:
    static TrayIcon *getInstance();
    static void setInstance(TrayIcon *trayIcon);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    explicit TrayIcon(QObject *parent = nullptr);
    ~TrayIcon();

public Q_SLOTS:
    void settingsChanged();
    void slotSetUnread(int unread);

private:
    static TrayIcon *m_instance;
    int m_unread = 0;
    bool mEnabled = false;
};
} // namespace Akregator

#endif // AKREGATOR_TRAYICON_H
