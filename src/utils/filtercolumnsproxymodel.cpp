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
    return source_column >= m_columnStates.size() ? m_mode == Blacklist : m_columnStates[source_column];
}

void FilterColumnsProxyModel::setColumnEnabled(int col, bool enabled)
{
    if (col >= m_columnStates.count()) {
        m_columnStates.resize(col + 1);
    }
    m_columnStates[col] = enabled;
}
