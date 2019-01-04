/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005-2007 Frank Osterfeld <osterfeld@kde.org>

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
#ifndef AKREGATOR_ARTICLELISTVIEW_H
#define AKREGATOR_ARTICLELISTVIEW_H

#include "akregatorpart_export.h"
#include "abstractselectioncontroller.h"

#include <QPointer>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include <QSharedPointer>
#include <QUrl>

class QContextMenuEvent;
template<class T> class QList;

namespace Akregator {
class Article;

namespace Filters {
}

class AKREGATORPART_EXPORT FilterDeletedProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:

    explicit FilterDeletedProxyModel(QObject *parent = nullptr);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

class AKREGATORPART_EXPORT SortColorizeProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:

    explicit SortColorizeProxyModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setFilters(const std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher> > &);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QIcon m_keepFlagIcon;
    std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher> > m_matchers;

    QColor m_unreadColor;
    QColor m_newColor;
};

class AKREGATORPART_EXPORT ArticleListView : public QTreeView, public ArticleLister
{
    Q_OBJECT

public:

    explicit ArticleListView(QWidget *parent = nullptr);
    ~ArticleListView() override;

    //impl ArticleLister
    void setArticleModel(Akregator::ArticleModel *model) override;

    //impl ArticleLister
    QItemSelectionModel *articleSelectionModel() const override;

    //impl ArticleLister
    const QAbstractItemView *itemView() const override;

    //impl ArticleLister
    QAbstractItemView *itemView() override;

    //impl ArticleLister
    QPoint scrollBarPositions() const override;

    //impl ArticleLister
    void setScrollBarPositions(const QPoint &p) override;

    //impl ArticleLister
    void setFilters(const std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher> > &) override;

    //impl ArticleLister
    void forceFilterUpdate() override;

    void setIsAggregation(bool isAggregation) override;

    void setModel(QAbstractItemModel *model) override;

protected:
    void mousePressEvent(QMouseEvent *ev) override;

    void paintEvent(QPaintEvent *event) override;

Q_SIGNALS:
    void signalMouseButtonPressed(int, const QUrl &);

public Q_SLOTS:

    void slotClear();

    void slotPreviousArticle();

    void slotNextArticle();

    void slotPreviousUnreadArticle();

    void slotNextUnreadArticle();

Q_SIGNALS:

    void userActionTakingPlace();

private:
    void restoreHeaderState();
    void saveHeaderSettings();
    void loadHeaderSettings();

    void contextMenuEvent(QContextMenuEvent *event) override;

    void selectIndex(const QModelIndex &index);

    void setFeedMode();
    void setGroupMode();

private Q_SLOTS:
    void showHeaderMenu(const QPoint &pos);
    void startResizingTitleColumn();
    void finishResizingTitleColumn();

private:
    void generalPaletteChanged();
    enum ColumnMode {
        GroupMode, FeedMode
    };
    ColumnMode m_columnMode;
    QColor mTextColor;
    QPointer<SortColorizeProxyModel> m_proxy;
    std::vector<QSharedPointer<const Filters::AbstractMatcher> > m_matchers;
    QByteArray m_feedHeaderState;
    QByteArray m_groupHeaderState;
};
} // namespace Akregator

#endif // AKREGATOR_ARTICLELISTVIEW_H
