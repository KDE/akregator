/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
#include "kcmutils_version.h"
#include <KCModule>

#include <QVariant>

namespace Akregator
{
class SettingsAdvanced;
}

class KCMAkregatorAdvancedConfig : public KCModule
{
    Q_OBJECT

public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KCMAkregatorAdvancedConfig(QWidget *parent, const QVariantList &args);
#else
    explicit KCMAkregatorAdvancedConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
#endif

    void load() override;
    void save() override;

private:
    Akregator::SettingsAdvanced *const m_widget;
};
