/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_TEMPORARYVALUE_H
#define AKREGATOR_TEMPORARYVALUE_H

namespace Akregator {
template<typename T>
class TemporaryValue
{
public:
    TemporaryValue(T &var_, const T &tempVal) : var(var_)
        , prev(var)
    {
        var = tempVal;
    }

    ~TemporaryValue()
    {
        var = prev;
    }

private:
    T &var;
    const T prev;
};
}

#endif // AKREGATOR_TEMPORARYVALUE_H
