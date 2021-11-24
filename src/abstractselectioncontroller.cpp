/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "abstractselectioncontroller.h"
using namespace Akregator;

AbstractSelectionController::AbstractSelectionController(QObject *parent)
    : QObject(parent)
{
}

AbstractSelectionController::~AbstractSelectionController() = default;
