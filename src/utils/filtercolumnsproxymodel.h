/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_FILTERCOLUMNSPROXYMODEL_H
#define AKREGATOR_FILTERCOLUMNSPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QVector>

namespace Akregator {
class FilterColumnsProxyModel : public QSortFilterProxyModel
{
public:
    enum Mode {
        Blacklist,
        Whitelist
    };

    explicit FilterColumnsProxyModel(QObject *parent = nullptr);

    void setColumnEnabled(int col, bool enabled = true);
    Q_REQUIRED_RESULT bool isColumnEnabled(int col) const;

    Q_REQUIRED_RESULT Mode mode() const;
    void setMode(Mode mode);

private:
    Q_REQUIRED_RESULT bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;

private:
    QVector<bool> m_columnStates;
    int m_vecSize = 0;
    Mode m_mode = Whitelist;
};
} // namespace Akregator

#endif // AKREGATOR_FILTERCOLUMNSPROXYMODEL_H
