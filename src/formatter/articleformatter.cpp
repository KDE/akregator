/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "articleformatter.h"
#include "akregatorconfig.h"
#include "article.h"
#include "feed.h"
#include "folder.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "utils.h"

#include <KLocalizedString>

#include <KFormat>

using namespace Syndication;
using namespace Akregator;

ArticleFormatter::ArticleFormatter() = default;

ArticleFormatter::~ArticleFormatter() = default;

QString ArticleFormatter::formatEnclosure(const Enclosure &enclosure)
{
    if (enclosure.isNull()) {
        return {};
    }

    const QString title = !enclosure.title().isEmpty() ? enclosure.title() : enclosure.url();
    const uint length = enclosure.length();
    const QString type = enclosure.type();
    QString inf;
    if (!type.isEmpty() && length > 0) {
        inf = i18n("(%1, %2)", type, KFormat().formatByteSize(length));
    } else if (!type.isNull()) {
        inf = type;
    } else if (length > 0) {
        inf = KFormat().formatByteSize(length);
    }
    const QString str = QStringLiteral("<a href=\"%1\">%2</a> %3").arg(enclosure.url(), title, inf);
    return str;
}
