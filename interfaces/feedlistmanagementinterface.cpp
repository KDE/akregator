/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "feedlistmanagementinterface.h"

using namespace Akregator;

FeedListManagementInterface *FeedListManagementInterface::m_instance = nullptr;

FeedListManagementInterface *FeedListManagementInterface::instance()
{
    return m_instance;
}

void FeedListManagementInterface::setInstance(FeedListManagementInterface *iface)
{
    m_instance = iface;
}

FeedListManagementInterface::~FeedListManagementInterface() = default;
