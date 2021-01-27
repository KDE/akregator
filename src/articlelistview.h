/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2005-2007 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#ifndef AKREGATOR_ARTICLELISTVIEW_H
#define AKREGATOR_ARTICLELISTVIEW_H

#include "abstractselectioncontroller.h"
#include "akregatorpart_export.h"

#include <QPointer>
#include <QSortFilterProxyModel>
#include <QTreeView>

#include <QSharedPointer>
#include <QUrl>

class QContextMenuEvent;
template<class T> class QList;

namespace Akregator
{
class Article;

namespace Filters
{
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

    Q_REQUIRED_RESULT QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setFilters(const std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher>> &);

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QIcon m_keepFlagIcon;
    std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher>> m_matchers;

    QColor m_unreadColor;
    QColor m_newColor;
};

class AKREGATORPART_EXPORT ArticleListView : public QTreeView, public ArticleLister
{
    Q_OBJECT

public:
    explicit ArticleListView(QWidget *parent = nullptr);
    ~ArticleListView() override;

    // impl ArticleLister
    void setArticleModel(Akregator::ArticleModel *model) override;

    // impl ArticleLister
    QItemSelectionModel *articleSelectionModel() const override;

    // impl ArticleLister
    const QAbstractItemView *itemView() const override;

    // impl ArticleLister
    QAbstractItemView *itemView() override;

    // impl ArticleLister
    Q_REQUIRED_RESULT QPoint scrollBarPositions() const override;

    // impl ArticleLister
    void setScrollBarPositions(const QPoint &p) override;

    // impl ArticleLister
    void setFilters(const std::vector<QSharedPointer<const Akregator::Filters::AbstractMatcher>> &) override;

    // impl ArticleLister
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
    enum ColumnMode { GroupMode, FeedMode };
    ColumnMode m_columnMode;
    QColor mTextColor;
    QPointer<SortColorizeProxyModel> m_proxy;
    std::vector<QSharedPointer<const Filters::AbstractMatcher>> m_matchers;
    QByteArray m_feedHeaderState;
    QByteArray m_groupHeaderState;
};
} // namespace Akregator

#endif // AKREGATOR_ARTICLELISTVIEW_H
