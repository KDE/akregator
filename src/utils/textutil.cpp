/*
   SPDX-FileCopyrightText: 2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "textutil.h"

// code from kitinerary/src/lib/stringutil.cpp
QChar TextUtil::normalize(QChar c)
{
    // case folding
    const auto n = c.toCaseFolded();

    // if the character has a canonical decomposition use that and skip the
    // combining diacritic markers following it
    // see https://en.wikipedia.org/wiki/Unicode_equivalence
    // see https://en.wikipedia.org/wiki/Combining_character
    if (n.decompositionTag() == QChar::Canonical) {
        return n.decomposition().at(0);
    }

    return n;
}

QString TextUtil::normalize(QStringView str)
{
    QString out;
    out.reserve(str.size());
    for (const auto c : str) {
        out.push_back(normalize(c));
    }
    return out;
}
