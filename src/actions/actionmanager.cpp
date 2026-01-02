/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "actionmanager.h"

using namespace Akregator;
ActionManager *ActionManager::m_self = nullptr;

ActionManager *ActionManager::getInstance()
{
    return m_self;
}

void ActionManager::setInstance(ActionManager *manager)
{
    m_self = manager;
}

ActionManager::ActionManager(QObject *parent)
    : QObject(parent)
{
}

ActionManager::~ActionManager() = default;

#include "moc_actionmanager.cpp"
