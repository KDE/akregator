/*
 *  This file is part of Akregator.
 *
 *  SPDX-FileCopyrightText: 2025 Richard de Ponte
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
 */

#include "systemsignalhandlers.h"
#ifdef USE_SYSTEM_SIGNAL_HANDLERS

#include <QCoreApplication>
#include <csignal>

static void GracefulQuit(int)
{
    qApp->quit();
}

void SystemSignalHandlers::RegisterTerminationSignalHandlers()
{
    const std::vector<int> terminationSignals{
        SIGHUP,
        SIGINT,
        SIGTERM,
    };

    for (const auto signal : terminationSignals) {
        ::signal(signal, GracefulQuit);
    }
}

#endif
