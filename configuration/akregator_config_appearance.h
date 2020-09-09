/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_CONFIG_APPEARANCE_H
#define AKREGATOR_CONFIG_APPEARANCE_H

#include "ui_settings_appearance.h"

#include <KCModule>

#include <QVariant>

class QWidget;

class KCMAkregatorAppearanceConfig : public KCModule
{
    Q_OBJECT
public:
    explicit KCMAkregatorAppearanceConfig(QWidget *parent, const QVariantList &args);

private:
    QWidget *const m_widget;
    Akregator::Ui::SettingsAppearance m_ui;
};

#endif // AKREGATOR_CONFIG_APPEARANCE_H
