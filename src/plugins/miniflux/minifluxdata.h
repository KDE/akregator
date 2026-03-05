/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2026 Blagovest Petrov <blagovest@petrovs.info>
    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <QDateTime>
#include <QList>
#include <QString>

namespace Akregator
{

struct MinifluxCategory {
    int id = 0;
    QString title;
};

struct MinifluxFeedData {
    int id = 0;
    int categoryId = 0;
    QString title;
    QString feedUrl;
    QString siteUrl;
    int unreadCount = 0;
};

struct MinifluxEntry {
    qint64 id = 0;
    int feedId = 0;
    QString title;
    QString url;
    QString author;
    QString content;
    QDateTime publishedAt;
    QString status; // "read" or "unread"
    bool starred = false;
};

} // namespace Akregator
