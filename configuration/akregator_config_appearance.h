/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "kcmutils_version.h"
#include "ui_settings_appearance.h"
#include <KCModule>

#include <QVariant>

class QWidget;

class KCMAkregatorAppearanceConfig : public KCModule
{
    Q_OBJECT
public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KCMAkregatorAppearanceConfig(QWidget *parent, const QVariantList &args);
#else
    explicit KCMAkregatorAppearanceConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
#endif

private:
    QWidget *const m_widget;
    Akregator::Ui::SettingsAppearance m_ui;
};
