/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_CONFIG_ADVANCED_H
#define AKREGATOR_CONFIG_ADVANCED_H

#include <KCModule>

#include <QVariant>

namespace Akregator {
class SettingsAdvanced;
}

class KCMAkregatorAdvancedConfig : public KCModule
{
    Q_OBJECT

public:
    KCMAkregatorAdvancedConfig(QWidget *parent, const QVariantList &args);

    void load() override;
    void save() override;

private:
    Akregator::SettingsAdvanced *const m_widget;
};

#endif
