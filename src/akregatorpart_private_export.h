/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akregatorpart_export.h"

/* Classes which are exported only for unit tests */
#ifdef BUILD_TESTING
#ifndef AKREGATORPART_TESTS_EXPORT
#define AKREGATORPART_TESTS_EXPORT AKREGATORPART_EXPORT
#endif
#else /* not compiling tests */
#define AKREGATORPART_TESTS_EXPORT
#endif
