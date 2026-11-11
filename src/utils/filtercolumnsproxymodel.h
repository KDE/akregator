/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include <QSortFilterProxyModel>

namespace Akregator
{
class FilterColumnsProxyModel : public QSortFilterProxyModel
{
public:
    enum Mode {
        Blacklist,
        Whitelist,
    };

    explicit FilterColumnsProxyModel(QObject *parent = nullptr);

    void setColumnEnabled(int col, bool enabled = true);

private:
    [[nodiscard]] bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;

private:
    QList<bool> m_columnStates;
    Mode m_mode = Whitelist;
};
} // namespace Akregator
