/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005-2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "settings_advanced.h"
#include "akregatorconfig.h"
#include <KLocalization>
#include <KLocalizedString>
#include <ki18n_version.h>

#include <QPushButton>
#include <QWidget>

#include <cassert>

using namespace Akregator;
SettingsAdvanced::SettingsAdvanced(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    connect(kcfg_UseMarkReadDelay, &QCheckBox::toggled, kcfg_MarkReadDelay, &QSpinBox::setEnabled);
#if KI18N_VERSION > QT_VERSION_CHECK(6, 5, 0)
    KLocalization::setupSpinBoxFormatString(kcfg_MarkReadDelay, ki18ncp("Mark selected article read after", " second", " seconds"));
#endif
}

#include "moc_settings_advanced.cpp"
