/*
    This file is part of Akregator.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_FILTERCOLUMNSPROXYMODEL_H
#define AKREGATOR_FILTERCOLUMNSPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QVector>

namespace Akregator
{

class FilterColumnsProxyModel : public QSortFilterProxyModel
{
public:
    enum Mode {
        Blacklist,
        Whitelist
    };

    explicit FilterColumnsProxyModel(QObject *parent = 0);

    void setColumnEnabled(int col, bool enabled = true);
    bool isColumnEnabled(int col) const;

    Mode mode() const;
    void setMode(Mode mode);

private:
    /*reimpl*/ bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;

private:
    QVector<bool> m_columnStates;
    int m_vecSize;
    Mode m_mode;
};

} // namespace Akregator

#endif // AKREGATOR_FILTERCOLUMNSPROXYMODEL_H
