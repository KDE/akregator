/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "accountplugin.h"

using namespace Akregator;

AccountPlugin::AccountPlugin(QObject *parent)
    : QObject(parent)
{
}

AccountPlugin::~AccountPlugin() = default;

#include "moc_accountplugin.cpp"
