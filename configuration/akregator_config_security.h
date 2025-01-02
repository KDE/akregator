/*
   SPDX-FileCopyrightText: 2021-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include <KCModule>

namespace Akregator
{
class KCMAkregatorSecurityConfig : public KCModule
{
    Q_OBJECT
public:
    explicit KCMAkregatorSecurityConfig(QObject *parent, const KPluginMetaData &data);

private:
    QWidget *const m_widget;
};
}
