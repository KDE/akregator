/*
   SPDX-FileCopyrightText: 2021-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include "kcmutils_version.h"
#include <KCModule>
#include <QVariant>

namespace Akregator
{
class KCMAkregatorSecurityConfig : public KCModule
{
    Q_OBJECT
public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KCMAkregatorSecurityConfig(QWidget *parent, const QVariantList &args);
#else
    explicit KCMAkregatorSecurityConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
#endif
private:
    QWidget *const m_widget;
};
}
