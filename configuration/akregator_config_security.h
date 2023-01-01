/*
   SPDX-FileCopyrightText: 2021-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KCModule>
#include <QVariant>

namespace Akregator
{
class KCMAkregatorSecurityConfig : public KCModule
{
    Q_OBJECT
public:
    explicit KCMAkregatorSecurityConfig(QWidget *parent, const QVariantList &args);

private:
    QWidget *const m_widget;
};
}
