/*
 *  This file is part of Akregator.
 *
 *  SPDX-FileCopyrightText: 2025 Richard de Ponte
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once
#include <qsystemdetection.h>
#if defined(Q_OS_UNIX)
#define USE_SYSTEM_SIGNAL_HANDLERS

namespace SystemSignalHandlers
{
void RegisterTerminationSignalHandlers();
}

#endif
