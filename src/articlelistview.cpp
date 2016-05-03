/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005-2008 Frank Osterfeld <osterfeld@kde.org>
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

#include "articlelistview.h"
#include "actionmanager.h"
#include "akregatorconfig.h"
#include "article.h"
#include "articlemodel.h"
#include "kernel.h"
#include "types.h"

#include <utils/filtercolumnsproxymodel.h>

#include <QDateTime>
#include <QIcon>
#include <KLocalizedString>
#include <QUrl>
#include <QMenu>
#include <KColorScheme>
#include <QLocale>

#include <QApplication>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QPaintEvent>
#include <QPalette>
#include <QScrollBar>

#include <cassert>

using namespace Akregator;

FilterDeletedProxyModel::FilterDeletedProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

bool FilterDeletedProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return !sourceModel()->index(source_row, 0, source_parent).data(ArticleModel::IsDeletedRole).toBool();
}

SortColorizeProxyModel::SortColorizeProxyModel(QObject *parent) : QSortFilterProxyModel(parent), m_keepFlagIcon(QIcon::fromTheme(QStringLiteral("mail-mark-important")))
{
    m_unreadColor = KColorScheme(QPalette::Normal, KColorScheme::View).foreground(KColorScheme::PositiveText).color();
    m_newColor = KColorScheme(QPalette::Normal, KColorScheme::View).foreground(KColorScheme::NegativeText).color();
}

bool SortColorizeProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (source_parent.isValid()) {
        return false;
    }

    for (uint i = 0; i < m_matchers.size(); ++i) {
        if (!static_cast<ArticleModel *>(sourceModel())->rowMatches(source_row, m_matchers[i])) {
            return false;
        }
    }

    return true;
}

void SortColorizeProxyModel::setFilters(const std::vector<QSharedPointer<const Filters::AbstractMatcher> >  &matchers)
{
    if (m_matchers == matchers) {
        return;
    }
    m_matchers = matchers;
    invalidateFilter();
}

QVariant SortColorizeProxyModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || !sourceModel()) {
        return QVariant();
    }

    const QModelIndex sourceIdx = mapToSource(idx);

    switch (role) {
    case Qt::ForegroundRole: {
        switch (static_cast<ArticleStatus>(sourceIdx.data(ArticleModel::StatusRole).toInt())) {
        case Unread: {
            return Settings::useCustomColors() ?
                   Settings::colorUnreadArticles() : m_unreadColor;
        }
        case New: {
            return Settings::useCustomColors() ?
                   Settings::colorNewArticles() : m_newColor;
        }
        case Read: {
            return QApplication::palette().color(QPalette::Text);
        }
        }
    }
    break;
    case Qt::DecorationRole: {
        if (sourceIdx.column() == ArticleModel::ItemTitleColumn) {
            return sourceIdx.data(ArticleModel::IsImportantRole).toBool() ? m_keepFlagIcon : QVariant();
        }
    }
    break;
    }
    return sourceIdx.data(role);
}

namespace
{

static bool isRead(const QModelIndex &idx)
{
    if (!idx.isValid()) {
        return false;
    }

    return static_cast<ArticleStatus>(idx.data(ArticleModel::StatusRole).toInt()) == Read;
}
}

void ArticleListView::setArticleModel(ArticleModel *model)
{
    if (!model) {
        setModel(model);
        return;
    }

    m_proxy = new SortColorizeProxyModel(model);
    m_proxy->setSourceModel(model);
    m_proxy->setSortRole(ArticleModel::SortRole);
    m_proxy->setFilters(m_matchers);
    FilterDeletedProxyModel *const proxy2 = new FilterDeletedProxyModel(model);
    proxy2->setSortRole(ArticleModel::SortRole);
    proxy2->setSourceModel(m_proxy);

    connect(model, &QAbstractItemModel::rowsInserted,
            m_proxy.data(), &QSortFilterProxyModel::invalidate);

    FilterColumnsProxyModel *const columnsProxy = new FilterColumnsProxyModel(model);
    columnsProxy->setSortRole(ArticleModel::SortRole);
    columnsProxy->setSourceModel(proxy2);
    columnsProxy->setColumnEnabled(ArticleModel::ItemTitleColumn);
    columnsProxy->setColumnEnabled(ArticleModel::FeedTitleColumn);
    columnsProxy->setColumnEnabled(ArticleModel::DateColumn);
    columnsProxy->setColumnEnabled(ArticleModel::AuthorColumn);

    setModel(columnsProxy);
    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    header()->setResizeMode(QHeaderView::Interactive);
}

void ArticleListView::showHeaderMenu(const QPoint &pos)
{
    if (!model()) {
        return;
    }

    QPointer<QMenu> menu = new QMenu(this);
    menu->setTitle(i18n("Columns"));
    menu->setAttribute(Qt::WA_DeleteOnClose);

    const int colCount = model()->columnCount();
    int visibleColumns = 0; // number of column currently shown
    QAction *visibleColumnsAction = 0;
    for (int i = 0; i < colCount; ++i) {
        QAction *act = menu->addAction(model()->headerData(i, Qt::Horizontal).toString());
        act->setCheckable(true);
        act->setData(i);
        bool sectionVisible = !header()->isSectionHidden(i);
        act->setChecked(sectionVisible);
        if (sectionVisible) {
            ++visibleColumns;
            visibleColumnsAction = act;
        }
    }

    // Avoid that the last shown column is also hidden
    if (visibleColumns == 1) {
        visibleColumnsAction->setEnabled(false);
    }

    QPointer<QObject> that(this);
    QAction *const action = menu->exec(header()->mapToGlobal(pos));
    if (that && action) {
        const int col = action->data().toInt();
        if (action->isChecked()) {
            header()->showSection(col);
        } else {
            header()->hideSection(col);
        }
    }
    delete menu;
}

void ArticleListView::saveHeaderSettings()
{
    if (model()) {
        const QByteArray state = header()->saveState();
        if (m_columnMode == FeedMode) {
            m_feedHeaderState = state;
        } else {
            m_groupHeaderState = state;
        }
    }

    KConfigGroup conf(Settings::self()->config(), "General");
    conf.writeEntry("ArticleListFeedHeaders", m_feedHeaderState.toBase64());
    conf.writeEntry("ArticleListGroupHeaders", m_groupHeaderState.toBase64());
}

void ArticleListView::loadHeaderSettings()
{
    KConfigGroup conf(Settings::self()->config(), "General");
    m_feedHeaderState = QByteArray::fromBase64(conf.readEntry("ArticleListFeedHeaders").toLatin1());
    m_groupHeaderState = QByteArray::fromBase64(conf.readEntry("ArticleListGroupHeaders").toLatin1());
}

QItemSelectionModel *ArticleListView::articleSelectionModel() const
{
    return selectionModel();
}

const QAbstractItemView *ArticleListView::itemView() const
{
    return this;
}

QAbstractItemView *ArticleListView::itemView()
{
    return this;
}

QPoint ArticleListView::scrollBarPositions() const
{
    return QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
}

void ArticleListView::setScrollBarPositions(const QPoint &p)
{
    horizontalScrollBar()->setValue(p.x());
    verticalScrollBar()->setValue(p.y());
}

void ArticleListView::setGroupMode()
{
    if (m_columnMode == GroupMode) {
        return;
    }

    if (model()) {
        m_feedHeaderState = header()->saveState();
    }
    m_columnMode = GroupMode;
    restoreHeaderState();
}

void ArticleListView::setFeedMode()
{
    if (m_columnMode == FeedMode) {
        return;
    }

    if (model()) {
        m_groupHeaderState = header()->saveState();
    }
    m_columnMode = FeedMode;
    restoreHeaderState();
}

static int maxDateColumnWidth(const QFontMetrics &fm)
{
    int width = 0;
    QDateTime date(QDate::currentDate(), QTime(23, 59));
    for (int x = 0; x < 10; ++x, date = date.addDays(-1)) {
        QString txt = QLatin1Char(' ') + QLocale().toString(date, QLocale::ShortFormat) + QLatin1Char(' ');
        width = qMax(width, fm.width(txt));
    }
    return width;
}

void ArticleListView::restoreHeaderState()
{
    QByteArray state = m_columnMode == GroupMode ? m_groupHeaderState : m_feedHeaderState;
    header()->restoreState(state);
    if (state.isEmpty()) {
        // No state, set a default config:
        // - hide the feed column in feed mode (no need to see the same feed title over and over)
        // - set the date column wide enough to fit all possible dates
        header()->setSectionHidden(ArticleModel::FeedTitleColumn, m_columnMode == FeedMode);
        header()->setStretchLastSection(false);
        header()->resizeSection(ArticleModel::DateColumn, maxDateColumnWidth(fontMetrics()));
        if (model()) {
            startResizingTitleColumn();
        }
    }

    if (header()->sectionSize(ArticleModel::DateColumn) == 1) {
        header()->resizeSection(ArticleModel::DateColumn, maxDateColumnWidth(fontMetrics()));
    }
}

void ArticleListView::startResizingTitleColumn()
{
    // set the title column to Stretch resize mode so that it adapts to the
    // content. finishResizingTitleColumn() will turn the resize mode back to
    // Interactive so that the user can still resize the column himself if he
    // wants to
    header()->setResizeMode(ArticleModel::ItemTitleColumn, QHeaderView::Stretch);
    QMetaObject::invokeMethod(this, "finishResizingTitleColumn", Qt::QueuedConnection);
}

void ArticleListView::finishResizingTitleColumn()
{
    if (QApplication::mouseButtons() != Qt::NoButton) {
        // Come back later: user is still resizing the widget
        QMetaObject::invokeMethod(this, "finishResizingTitleColumn", Qt::QueuedConnection);
        return;
    }
    header()->setResizeMode(QHeaderView::Interactive);
}

ArticleListView::~ArticleListView()
{
    saveHeaderSettings();
}

void ArticleListView::setIsAggregation(bool aggregation)
{
    if (aggregation) {
        setGroupMode();
    } else {
        setFeedMode();
    }
}

ArticleListView::ArticleListView(QWidget *parent)
    : QTreeView(parent),
      m_columnMode(FeedMode)
{
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setUniformRowHeights(true);
    setRootIsDecorated(false);
    setAllColumnsShowFocus(true);
    setDragDropMode(QAbstractItemView::DragOnly);

    setMinimumSize(250, 150);
    setWhatsThis(i18n("<h2>Article list</h2>"
                      "Here you can browse articles from the currently selected feed. "
                      "You can also manage articles, as marking them as persistent (\"Mark as Important\") or delete them, using the right mouse button menu. "
                      "To view the web page of the article, you can open the article internally in a tab or in an external browser window."));

    //connect exactly once
    disconnect(header(), &QWidget::customContextMenuRequested, this, &ArticleListView::showHeaderMenu);
    connect(header(), &QWidget::customContextMenuRequested, this, &ArticleListView::showHeaderMenu);
    loadHeaderSettings();
}

void ArticleListView::mousePressEvent(QMouseEvent *ev)
{
    // let's push the event, so we can use currentIndex() to get the newly selected article..
    QTreeView::mousePressEvent(ev);

    if (ev->button() == Qt::MidButton) {
        const QUrl url = currentIndex().data(ArticleModel::LinkRole).toUrl();

        Q_EMIT signalMouseButtonPressed(ev->button(), url);
    }
}

void ArticleListView::contextMenuEvent(QContextMenuEvent *event)
{
    QWidget *w = ActionManager::getInstance()->container(QStringLiteral("article_popup"));
    QMenu *popup = qobject_cast<QMenu *>(w);
    if (popup) {
        popup->exec(event->globalPos());
    }
}

void ArticleListView::setModel(QAbstractItemModel *m)
{
    const bool groupMode = m_columnMode == GroupMode;

    QAbstractItemModel *const oldModel = model();
    if (oldModel) {
        const QByteArray state = header()->saveState();
        if (groupMode) {
            m_groupHeaderState = state;
        } else {
            m_feedHeaderState = state;
        }
    }

    QTreeView::setModel(m);

    if (m) {
        sortByColumn(ArticleModel::DateColumn, Qt::DescendingOrder);
        restoreHeaderState();

        // Ensure at least one column is visible
        if (header()->hiddenSectionCount() == header()->count()) {
            header()->showSection(ArticleModel::ItemTitleColumn);
        }
    }
}

void ArticleListView::slotClear()
{
    setModel(Q_NULLPTR);
}

void ArticleListView::slotPreviousArticle()
{
    if (!model()) {
        return;
    }
    Q_EMIT userActionTakingPlace();
    const QModelIndex idx = currentIndex();
    const int newRow = qMax(0, (idx.isValid() ? idx.row() : model()->rowCount()) - 1);
    const QModelIndex newIdx = idx.isValid() ? idx.sibling(newRow, 0) : model()->index(newRow, 0);
    selectIndex(newIdx);
}

void ArticleListView::slotNextArticle()
{
    if (!model()) {
        return;
    }

    Q_EMIT userActionTakingPlace();
    const QModelIndex idx = currentIndex();
    const int newRow = idx.isValid() ? (idx.row() + 1) : 0;
    const QModelIndex newIdx = model()->index(qMin(newRow, model()->rowCount() - 1), 0);
    selectIndex(newIdx);
}

void ArticleListView::slotNextUnreadArticle()
{
    if (!model()) {
        return;
    }

    const int rowCount = model()->rowCount();
    const int startRow = qMin(rowCount - 1, (currentIndex().isValid() ? currentIndex().row() + 1 : 0));

    int i = startRow;
    bool foundUnread = false;

    do {
        if (!::isRead(model()->index(i, 0))) {
            foundUnread = true;
        } else {
            i = (i + 1) % rowCount;
        }
    } while (!foundUnread && i != startRow);

    if (foundUnread) {
        selectIndex(model()->index(i, 0));
    }
}

void ArticleListView::selectIndex(const QModelIndex &idx)
{
    if (!idx.isValid()) {
        return;
    }
    setCurrentIndex(idx);
    clearSelection();
    Q_ASSERT(selectionModel());
    selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    scrollTo(idx, PositionAtCenter);
}

void ArticleListView::slotPreviousUnreadArticle()
{
    if (!model()) {
        return;
    }

    const int rowCount = model()->rowCount();
    const int startRow = qMax(0, (currentIndex().isValid() ? currentIndex().row() : rowCount) - 1);

    int i = startRow;
    bool foundUnread = false;

    do {
        if (!::isRead(model()->index(i, 0))) {
            foundUnread = true;
        } else {
            i = i > 0 ? i - 1 : rowCount - 1;
        }
    } while (!foundUnread && i != startRow);

    if (foundUnread) {
        selectIndex(model()->index(i, 0));
    }
}

void ArticleListView::forceFilterUpdate()
{
    if (m_proxy) {
        m_proxy->invalidate();
    }
}

void ArticleListView::setFilters(const std::vector<QSharedPointer<const Filters::AbstractMatcher> > &matchers)
{
    if (m_matchers == matchers) {
        return;
    }
    m_matchers = matchers;
    if (m_proxy) {
        m_proxy->setFilters(matchers);
    }
}

