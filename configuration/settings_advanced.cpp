/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005-2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "settings_advanced.h"
#include "akregatorconfig.h"
#include <KLocalization>
#include <KLocalizedString>

#include <QPushButton>
#include <QWidget>

#include <cassert>

using namespace Akregator;
SettingsAdvanced::SettingsAdvanced(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    connect(kcfg_UseMarkReadDelay, &QCheckBox::toggled, kcfg_MarkReadDelay, &QSpinBox::setEnabled);
    KLocalization::setupSpinBoxFormatString(kcfg_MarkReadDelay, ki18ncp("Mark selected article read after", "%v second", "%v seconds"));
}

#include "moc_settings_advanced.cpp"
