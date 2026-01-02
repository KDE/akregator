/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "filtercolumnsproxymodel.h"

using namespace Akregator;

FilterColumnsProxyModel::FilterColumnsProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool FilterColumnsProxyModel::filterAcceptsColumn(int source_column, const QModelIndex &) const
{
    return source_column >= m_vecSize ? m_mode == Blacklist : m_columnStates[source_column];
}

void FilterColumnsProxyModel::setColumnEnabled(int col, bool enabled)
{
    if (col >= m_columnStates.count()) {
        m_columnStates.resize(col + 1);
        m_vecSize = col + 1;
    }
    m_columnStates[col] = enabled;
}

bool FilterColumnsProxyModel::isColumnEnabled(int col) const
{
    Q_ASSERT(col >= 0);
    return col < m_columnStates.count() ? m_columnStates[col] : m_mode == Blacklist;
}

FilterColumnsProxyModel::Mode FilterColumnsProxyModel::mode() const
{
    return m_mode;
}

void FilterColumnsProxyModel::setMode(Mode mode)
{
    m_mode = mode;
}
