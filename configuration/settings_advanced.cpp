/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005-2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "settings_advanced.h"
#include "akregatorconfig.h"

#include <KLocalizedString>

#include <QPushButton>
#include <QWidget>

#include <cassert>

using namespace Akregator;
SettingsAdvanced::SettingsAdvanced(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    connect(kcfg_UseMarkReadDelay, &QCheckBox::toggled, kcfg_MarkReadDelay, &KPluralHandlingSpinBox::setEnabled);

    kcfg_MarkReadDelay->setSuffix(ki18ncp("Mark selected article read after", " second", " seconds"));
}
