/*
 * shared.h
 *
 * SPDX-FileCopyrightText: 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef AKREGATOR_SHARED_H
#define AKREGATOR_SHARED_H

namespace Akregator {
struct Shared {
    Shared() : count(1)
    {
    }

    void ref()
    {
        count++;
    }

    bool deref()
    {
        return !--count;
    }

    unsigned int count;
};
}

#endif // AKREGATOR_SHARED_H
