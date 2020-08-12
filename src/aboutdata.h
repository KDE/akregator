/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Teemu Rytilahti <tpr@d5k.net>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_ABOUTDATA_H
#define AKREGATOR_ABOUTDATA_H

#include "akregator_export.h"
#include <KAboutData>

namespace Akregator {
/**
@author Teemu Rytilahti
*/
class AKREGATOR_EXPORT AboutData : public KAboutData
{
public:
    AboutData();
    ~AboutData();
};
} // namespace Akregator

#endif // AKREGATOR_ABOUTDATA_H
