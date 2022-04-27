/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005-2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "ui_settings_advancedbase.h"

#include <QHash>

class QString;
class QWidget;

namespace Akregator
{
namespace Backend
{
class StorageFactory;
}

class SettingsAdvanced : public QWidget, public Ui::SettingsAdvancedBase
{
    Q_OBJECT

public:
    explicit SettingsAdvanced(QWidget *parent = nullptr);
};
} // namespace Akregator
