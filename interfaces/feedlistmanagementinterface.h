/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregatorinterfaces_export.h"

class QString;
#include <QStringList>

namespace Akregator
{
class AKREGATORINTERFACES_EXPORT FeedListManagementInterface
{
public:
    static FeedListManagementInterface *instance();
    static void setInstance(FeedListManagementInterface *);

    virtual ~FeedListManagementInterface();

    [[nodiscard]] virtual QStringList categories() const = 0;
    [[nodiscard]] virtual QStringList feeds(const QString &catId) const = 0;
    virtual void addFeed(const QString &url, const QString &catId) = 0;
    virtual void removeFeed(const QString &url, const QString &catId) = 0;
    [[nodiscard]] virtual QString getCategoryName(const QString &catId) const = 0;

private:
    static FeedListManagementInterface *m_instance;
};
}
