/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregator_export.h"
#include <QString>
using uint = unsigned int;

namespace Akregator
{
class AKREGATOR_EXPORT Utils
{
public:
    /** removes HTML/XML tags (everything between &lt; and &gt;) from a string.  "<p><strong>foo</strong> bar</p>" becomes "foo bar" */
    static QString stripTags(QString str);

    /** taken from some website... -fo
     * djb2
     * This algorithm was first reported by Dan Bernstein
     * many years ago in comp.lang.c
     */

    static uint calcHash(const QString &str);

    static QString convertHtmlTags(const QString &title);
};
} // namespace Akregator

