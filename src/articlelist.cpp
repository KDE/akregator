/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "akregatorconfig.h"
#include "articlelist.h"
#include "feed.h"
#include "treenode.h"

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcharsets.h>

#include <qdatetime.h>
#include <qpixmap.h>
#include <qwhatsthis.h>
#include <qheader.h>

using namespace Akregator;
using namespace RSS;

ArticleListItem::ArticleListItem( QListView *parent, QListViewItem *after, const MyArticle& a, Feed *feed)
    : KListViewItem( parent, after, KCharsets::resolveEntities(a.title()), feed->title(), KGlobal::locale()->formatDateTime(a.pubDate(), true, false) )
{
    m_article = a;
    m_feed = feed;
    if (a.keep())
        setPixmap(0, QPixmap(locate("data", "akregator/pics/akregator_flag.png")));
}
 
ArticleListItem::~ArticleListItem()
{
}


MyArticle& ArticleListItem::article()
{
    return m_article;
}

Feed *ArticleListItem::feed()
{
    return m_feed;
}

// paint ze peons
void ArticleListItem::paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
    QColorGroup cg2(cg);
    
    // XXX: make configurable
    if (article().status()==MyArticle::Unread)
        cg2.setColor(QColorGroup::Text, Qt::blue);
    else if (article().status()==MyArticle::New)
        cg2.setColor(QColorGroup::Text, Qt::red);
    
    KListViewItem::paintCell( p, cg2, column, width, align );

}


int ArticleListItem::compare(QListViewItem *i, int col, bool ascending) const {
    if (col == 2) {
        ArticleListItem *item = static_cast<ArticleListItem*>(i);
        if (item && item->m_article.pubDate().isValid() && m_article.pubDate().isValid()) {
            return ascending ?
		    item->m_article.pubDate().secsTo(m_article.pubDate()) :
		    -m_article.pubDate().secsTo(item->m_article.pubDate());
        }
    }
    return KListViewItem::compare(i, col, ascending);
}

/* ==================================================================================== */

ArticleList::ArticleList(QWidget *parent, const char *name)
    : KListView(parent, name), m_updated(false), m_doReceive(true), m_node(0), m_columnMode(feedMode)
{
    setMinimumSize(250, 150);
    addColumn(i18n("Article"));
    addColumn(i18n("Feed"));
    addColumn(i18n("Date"));
    setColumnWidthMode(2, QListView::Maximum);
    setColumnWidthMode(1, QListView::Manual);
    setColumnWidthMode(0, QListView::Manual);
    setRootIsDecorated(false);
    setItemsRenameable(false);
    setItemsMovable(false);
    setAllColumnsShowFocus(true);
    setDragEnabled(false); // FIXME before we implement dragging between archived feeds??
    setAcceptDrops(false); // FIXME before we implement dragging between archived feeds??
    setFullWidth(false);
    setShowSortIndicator(true);
    setDragAutoScroll(true);
    setDropHighlighter(false);

    int c = Settings::sortColumn();
    setSorting((c >= 0 && c <= 2) ? c : 2, Settings::sortAscending());

    int w;
    w = Settings::titleWidth();
    if (w > 0) {
        setColumnWidth(0, w);
    }
    
    w = Settings::feedWidth();
    if (w > 0) {
        setColumnWidth(1, w);
    }
    
    w = Settings::dateWidth();
    if (w > 0) {
        setColumnWidth(2, w);
    }
    
    m_feedWidth = columnWidth(1);
    hideColumn(1);

    header()->setStretchEnabled(true, 0);

    QWhatsThis::add(this, i18n("<h2>Article list</h2>"
        "Here you can browse articles from the currently selected feed. "
        "You can also manage articles, as marking them as persistent (\"Keep Article\") or delete them, using the right mouse button menu."
        "To view the web page of the article, you can open the article internally in a tab or in an external browser window."));

    connect(this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)) );
    connect(this, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),  this, SLOT(slotDoubleClicked(QListViewItem*, const QPoint&, int)) );
    connect(this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
            this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)));
}
void ArticleList::slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter)
{
    if ( (textFilter != m_textFilter) || (statusFilter != m_statusFilter) )
    {
        m_textFilter = textFilter;
        m_statusFilter = statusFilter;
               
        applyFilters();
    }
}

void ArticleList::setReceiveUpdates(bool doReceive, bool remember)
{
    if (m_doReceive && !doReceive)
    {    
        m_updated = false;  
        m_doReceive = false;
        return;
    }
    
    if (!m_doReceive && doReceive)
    {    
        m_doReceive = true;
        if (remember && m_updated)
            slotUpdate();
        m_updated = false;  
    }   
}

void ArticleList::slotShowNode(TreeNode* node)
{
    if (!node)
        return slotClear();
     
    if (m_node)
    {
        disconnect(m_node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdate()) );
        disconnect(m_node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear()) );
    }
    
    connect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdate()) );
    connect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear()) );

    m_node = node;
        
    clear();
    
    if ( node->isGroup() && m_columnMode == feedMode )
    {
        setColumnWidth(1, m_feedWidth);
        m_columnMode = groupMode;
    }
    else if ( !node->isGroup() && m_columnMode == groupMode)
    {    
        m_feedWidth = columnWidth(1);
        hideColumn(1);
        m_columnMode = feedMode;
    }

    slotUpdate();
}

void ArticleList::slotClear()
{
    if (m_node)
    {
        disconnect(m_node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdate()) );
        disconnect(m_node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear()) );
    }
    m_node = 0;
    
    clear();
}

void ArticleList::slotUpdate()
{
    if (!m_doReceive)
    {
        m_updated = true;
        return;
    }
        
    if (!m_node) 
        return;    
    
    setUpdatesEnabled(false);

    MyArticle oldCurrentArticle;
    ArticleListItem *li = dynamic_cast<ArticleListItem*>(selectedItem());
    bool haveOld = false;
    if (li) {
        oldCurrentArticle = li->article();
        haveOld = true;
    }

    clear();
    
    ArticleSequence articles = m_node->articles();
    
    ArticleSequence::ConstIterator end = articles.end();
    ArticleSequence::ConstIterator it = articles.begin();

    setShowSortIndicator(false);
    int col = sortColumn();
    SortOrder order = sortOrder();
    setSorting(-1);

    for (; it != end; ++it) {
        if (!(*it).isDeleted()) {
            ArticleListItem *ali = new ArticleListItem(this, lastChild(), *it, (*it).feed());
            if (haveOld && *it == oldCurrentArticle) {
                setCurrentItem(ali);
                setSelected(ali, true);
                haveOld = false;
            }
        }
    }

    setSorting(col, order == Ascending);
    setShowSortIndicator(true);

    applyFilters();        
    setUpdatesEnabled(true);
    triggerUpdate();
}

void ArticleList::applyFilters()
{
    ArticleListItem* ali = 0;
    for (QListViewItemIterator it(this); it.current(); ++it)
    {
        ali = static_cast<ArticleListItem*> (it.current());
        ali->setVisible( m_statusFilter.matches( ali->article() ) && m_textFilter.matches( ali->article() ) );
    }
}

void ArticleList::slotPreviousArticle()
{
    QListViewItem *lvi = selectedItem();
    
    if (!lvi && firstChild() )
    {
        setSelected(firstChild(), true);
    }
    
    if (lvi && lvi->itemAbove()) 
    {
        setSelected( lvi->itemAbove(), true );
        ensureItemVisible(lvi->itemAbove());
    }
}

void ArticleList::slotNextArticle()
{
    QListViewItem *lvi = selectedItem();
    
    if (!lvi && firstChild() )
    {
        setSelected(firstChild(), true);
        return;
    }
    if (lvi && lvi->itemBelow()) 
    {
        setSelected( lvi->itemBelow(), true );
        ensureItemVisible(lvi->itemBelow());
    }
}

void ArticleList::slotNextUnreadArticle()
{
    ArticleListItem *it= static_cast<ArticleListItem*>(selectedItem());
    if (!it)
        it = static_cast<ArticleListItem*>(firstChild());

    for ( ; it; it = static_cast<ArticleListItem*>(it->nextSibling()))
    {
        if (it->article().status() != MyArticle::Read)
        {
            setSelected(it, true);
            ensureItemVisible(it);
            return;
        }
    }
    // only reached when there is no unread article after the selected one
    if (m_node->unread() > 0)
    {
        it = static_cast<ArticleListItem*>(firstChild());
        for ( ; it; it = static_cast<ArticleListItem*>(it->nextSibling()))
        {
            if (it->article().status() != MyArticle::Read)
            {
                setSelected(it, true);
                ensureItemVisible(it);
                return;
            }
        }
    }
}

void ArticleList::slotPreviousUnreadArticle()
{
    if ( !selectedItem() )
        slotNextUnreadArticle(); 

    QListViewItemIterator it( selectedItem() );
    
    for ( ; it.current(); --it )
    {
        ArticleListItem* ali = static_cast<ArticleListItem*> (it.current());
        if (!ali)
            break;
        if ((ali->article().status()==MyArticle::Unread) ||
             (ali->article().status()==MyArticle::New))
        {
            setSelected(ali, true);
            ensureItemVisible(ali);
            return;
        }
    }
    // only reached when there is no unread article before the selected one
    if (m_node->unread() > 0)
    {
        it = static_cast<ArticleListItem*>(lastChild());

        for ( ; it.current(); --it )
        {
            ArticleListItem* ali = static_cast<ArticleListItem*> (it.current());
            if ((ali->article().status() != MyArticle::Read))
            {
                setSelected(ali, true);
                ensureItemVisible(ali);
                return;
            }
        }
    }
}

void ArticleList::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}

void ArticleList::slotSelectionChanged(QListViewItem* item)
{
    ArticleListItem* ai = static_cast<ArticleListItem*> (item);
    if (ai)
        emit signalArticleSelected( ai->article() );
} 

void ArticleList::slotDoubleClicked(QListViewItem* item, const QPoint& p, int i)
{
    emit signalDoubleClicked(static_cast<ArticleListItem*>(item), p, i);
}

void ArticleList::slotContextMenu(KListView* list, QListViewItem* item, const QPoint& p)
{
    emit signalContextMenu(list, static_cast<ArticleListItem*>(item), p);
}
        
ArticleList::~ArticleList()
{
    Settings::setTitleWidth(columnWidth(0));
    Settings::setFeedWidth(columnWidth(1) > 0 ? columnWidth(1) : m_feedWidth);
    Settings::setSortColumn(sortColumn());
    Settings::setSortAscending(sortOrder() == Ascending);
    Settings::writeConfig();
}

#include "articlelist.moc"
// vim: ts=4 sw=4 et
