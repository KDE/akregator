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
#include "actionmanager.h"
#include "articlelistview.h"
#include "article.h"
#include "feed.h"
#include "treenode.h"
#include "treenodevisitor.h"

#include <kstandarddirs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kurl.h>

#include <qdatetime.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qwhatsthis.h>
#include <qheader.h>
#include <qdragobject.h>
#include <qsimplerichtext.h>
#include <qpainter.h>
#include <qapplication.h>

namespace Akregator {

class ArticleListView::ColumnLayoutVisitor : public TreeNodeVisitor
{
    public:
        ColumnLayoutVisitor(ArticleListView* view) : m_view(view) {}

        virtual bool visitTagNode(TagNode* /*node*/)
        {
            if (m_view->m_columnMode == ArticleListView::feedMode)
            {
                m_view->setColumnWidth(1, m_view->m_feedWidth);
                m_view->m_columnMode = ArticleListView::groupMode;
            }
            return true;
        }
        
        virtual bool visitFolder(Folder* /*node*/)
        {
            if (m_view->m_columnMode == ArticleListView::feedMode)
            {
                m_view->setColumnWidth(1, m_view->m_feedWidth);
                m_view->m_columnMode = ArticleListView::groupMode;
            }
            return true;
        }
        
        virtual bool visitFeed(Feed* /*node*/)
        {
            if (m_view->m_columnMode == ArticleListView::groupMode)
            {    
                m_view->m_feedWidth = m_view->columnWidth(1);
                m_view->hideColumn(1);
                m_view->m_columnMode = ArticleListView::feedMode;
            }
            return true;
        }
    private:

        ArticleListView* m_view;
    
};

// FIXME: Remove resolveEntities for KDE 4.0, it's now done in the parser
ArticleItem::ArticleItem( QListView *parent, const Article& a, Feed *feed)
    : KListViewItem( parent, KCharsets::resolveEntities(a.title()), feed->title(), KGlobal::locale()->formatDateTime(a.pubDate(), true, false) ), m_article(a), m_feed(feed), m_pubDate(a.pubDate().toTime_t())
{
    if (a.keep())
        setPixmap(0, m_keepFlag);
}
 
ArticleItem::~ArticleItem()
{
}

Article& ArticleItem::article()
{
    return m_article;
}

QPixmap ArticleItem::m_keepFlag = QPixmap(locate("data", "akregator/pics/akregator_flag.png"));

Feed *ArticleItem::feed()
{
    return m_feed;
}

// paint ze peons
void ArticleItem::paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
    if (article().status() == Article::Read)
        KListViewItem::paintCell( p, cg, column, width, align );
    else
    {
        // if article status is unread or new, we change the color: FIXME: make colors configurable
        QColorGroup cg2(cg);
    
        if (article().status() == Article::Unread)
            cg2.setColor(QColorGroup::Text, Qt::blue);
        else // New
            cg2.setColor(QColorGroup::Text, Qt::red);
    
        KListViewItem::paintCell( p, cg2, column, width, align );
    }

}

int ArticleItem::compare(QListViewItem *i, int col, bool ascending) const {
    if (col == 2)
    {
        ArticleItem* item = static_cast<ArticleItem*>(i);
        if (m_pubDate == item->m_pubDate)
            return 0;
        return (m_pubDate > item->m_pubDate) ? 1 : -1;
    }
    return KListViewItem::compare(i, col, ascending);
}

/* ==================================================================================== */

ArticleListView::ArticleListView(QWidget *parent, const char *name)
    : KListView(parent, name), m_updated(false), m_doReceive(true), m_node(0), m_columnMode(feedMode)
{
    m_columnLayoutVisitor = new ColumnLayoutVisitor(this);
    setMinimumSize(250, 150);
    addColumn(i18n("Article"));
    addColumn(i18n("Feed"));
    addColumn(i18n("Date"));
    setSelectionMode(QListView::Extended);
    setColumnWidthMode(2, QListView::Maximum);
    setColumnWidthMode(1, QListView::Manual);
    setColumnWidthMode(0, QListView::Manual);
    setRootIsDecorated(false);
    setItemsRenameable(false);
    setItemsMovable(false);
    setAllColumnsShowFocus(true);
    setDragEnabled(true); // FIXME before we implement dragging between archived feeds??
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

    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );
    connect(this, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),  this, SLOT(slotDoubleClicked(QListViewItem*, const QPoint&, int)) );
    connect(this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
            this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)));
}
void ArticleListView::slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter)
{
    if ( (textFilter != m_textFilter) || (statusFilter != m_statusFilter) )
    {
        m_textFilter = textFilter;
        m_statusFilter = statusFilter;
               
        applyFilters();
    }
}

void ArticleListView::setReceiveUpdates(bool doReceive, bool remember)
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

void ArticleListView::slotShowNode(TreeNode* node)
{
    if (!node)
    {
        slotClear();
        return;
    }
     
    if (m_node)
        disconnectFromNode(m_node);

    connectToNode(node);

    m_node = node;
        
    clear();
    
    m_columnLayoutVisitor->visit(node);

    slotUpdate();
}

void ArticleListView::slotClear()
{
    if (m_node)
        disconnectFromNode(m_node);
        
    m_node = 0;
    m_articleMap.clear();
    clear();
}

void ArticleListView::slotArticlesAdded(TreeNode* /*node*/, const QValueList<Article>& list)
{
    setUpdatesEnabled(false);
    for (QValueList<Article>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        if (!m_articleMap.contains(*it))
        {
            if (!(*it).isDeleted())
            {
                ArticleItem* ali = new ArticleItem(this, *it, (*it).feed());
                ali->setVisible( m_textFilter.matches( ali->article()) );
                m_articleMap.insert(*it, ali);
            }
        }
    }
    setUpdatesEnabled(true);
    triggerUpdate();
}

void ArticleListView::slotArticlesUpdated(TreeNode* /*node*/, const QValueList<Article>& list)
{
    setUpdatesEnabled(false);
    for (QValueList<Article>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        ArticleItem* ali = m_articleMap[*it];
        if (ali)
        {
            if (ali->article().isDeleted()) // if article was set to deleted, delete item
            {
                m_articleMap.remove(*it);
                delete ali;
            }
            else
            {
                bool isCurrent = currentItem() == ali;
                bool isSelected = ali->isSelected();

                delete ali;
                ali = new ArticleItem(this, *it, (*it).feed());
                m_articleMap[*it] = ali;
                // set visibility depending on text filter. we ignore status filter here, as we don't want articles to vanish when selected with quick filter set to "Unread" 
                if (m_textFilter.matches( ali->article()))
                {
                    if (isCurrent)
                        setCurrentItem(ali);
                    setSelected(ali, isSelected);
                }
                else
                    ali->setVisible(false);
                
            }
        }
    }
    setUpdatesEnabled(true);
    triggerUpdate();
}

void ArticleListView::slotArticlesRemoved(TreeNode* /*node*/, const QValueList<Article>& list)
{
    setUpdatesEnabled(false);
    for (QValueList<Article>::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        if (m_articleMap.contains(*it))
        {
            ArticleItem* ali = m_articleMap[*it];
            m_articleMap.remove(*it);
            delete ali;
        }
    }
    setUpdatesEnabled(true);
    triggerUpdate();
}
            
void ArticleListView::slotUpdate()
{
    if (!m_doReceive)
    {
        m_updated = true;
        return;
    }
        
    if (!m_node) 
        return;    
    
    setUpdatesEnabled(false);

    Article oldCurrentArticle;
    
    ArticleItem *li = dynamic_cast<ArticleItem*>(currentItem());
    bool haveOld = false;
    if (li)
    {
        oldCurrentArticle = li->article();
        haveOld = true;
    }
    
    QPtrList<QListViewItem> selItems = selectedItems(false);

    QValueList<Article> selectedArticles;

    int haveSelected = 0;    
    for (QListViewItem* i = selItems.first(); i; i = selItems.next() )
    {
        ArticleItem* si = static_cast<ArticleItem*>(i);
        selectedArticles.append(si->article());
        ++haveSelected;
    }
    
    clear();
    
    setShowSortIndicator(false);
    int col = sortColumn();
    SortOrder order = sortOrder();
    setSorting(-1);

    QValueList<Article> articles = m_node->articles();

    QValueList<Article>::ConstIterator end = articles.end();
    QValueList<Article>::ConstIterator it = articles.begin();
    
    for (; it != end; ++it)
    {
        if (!(*it).isDeleted())
        {
            ArticleItem *ali = new ArticleItem(this, *it, (*it).feed());
            m_articleMap.insert(*it, ali);
            if (haveOld && *it == oldCurrentArticle)
            {
                setCurrentItem(ali);
                haveOld = false;
            }
            if (haveSelected > 0 && selectedArticles.contains(ali->article()))
            {
                setSelected(ali, true);
                --haveSelected;
            }
        }
    }
    
    setSorting(col, order == Ascending);
    setShowSortIndicator(true);

    applyFilters();
    setUpdatesEnabled(true);
    triggerUpdate();
}

void ArticleListView::connectToNode(TreeNode* node)
{
    //connect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdate()) );
    connect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear()) );
    connect(node, SIGNAL(signalArticlesAdded(TreeNode*, const QValueList<Article>&)), this, SLOT(slotArticlesAdded(TreeNode*, const QValueList<Article>&)) );
    connect(node, SIGNAL(signalArticlesUpdated(TreeNode*, const QValueList<Article>&)), this, SLOT(slotArticlesUpdated(TreeNode*, const QValueList<Article>&)) );
    connect(node, SIGNAL(signalArticlesRemoved(TreeNode*, const QValueList<Article>&)), this, SLOT(slotArticlesRemoved(TreeNode*, const QValueList<Article>&)) );
}

void ArticleListView::disconnectFromNode(TreeNode* node)
{
    //disconnect(node, SIGNAL(signalChanged(TreeNode*)), this, SLOT(slotUpdate()) );
    disconnect(node, SIGNAL(signalDestroyed(TreeNode*)), this, SLOT(slotClear()) );
    disconnect(node, SIGNAL(signalArticlesAdded(TreeNode*, const QValueList<Article>&)), this, SLOT(slotArticlesAdded(TreeNode*, const QValueList<Article>&)) );
    disconnect(node, SIGNAL(signalArticlesUpdated(TreeNode*, const QValueList<Article>&)), this, SLOT(slotArticlesUpdated(TreeNode*, const QValueList<Article>&)) );
    disconnect(node, SIGNAL(signalArticlesRemoved(TreeNode*, const QValueList<Article>&)), this, SLOT(slotArticlesRemoved(TreeNode*, const QValueList<Article>&)) );
}

void ArticleListView::applyFilters()
{
    ArticleItem* ali = 0;
    for (QListViewItemIterator it(this); it.current(); ++it)
    {
        ali = static_cast<ArticleItem*> (it.current());
        ali->setVisible( m_statusFilter.matches( ali->article() ) && m_textFilter.matches( ali->article() ) );
    }
}

int ArticleListView::visibleArticles()
{
    int visible = 0;
    ArticleItem* ali = 0;
    for (QListViewItemIterator it(this); it.current(); ++it) {
        ali = static_cast<ArticleItem*> (it.current());
        visible += ali->isVisible() ? 1 : 0;
    }
    return visible;
}

// from amarok :)
void ArticleListView::paintInfoBox(const QString &message)
{
    QPainter p( viewport() );
    QSimpleRichText t( message, QApplication::font() );

    if ( t.width()+30 >= viewport()->width() || t.height()+30 >= viewport()->height() )
            //too big, giving up
        return;

    const uint w = t.width();
    const uint h = t.height();
    const uint x = (viewport()->width() - w - 30) / 2 ;
    const uint y = (viewport()->height() - h - 30) / 2 ;

    p.setBrush( colorGroup().background() );
    p.drawRoundRect( x, y, w+30, h+30, (8*200)/w, (8*200)/h );
    t.draw( &p, x+15, y+15, QRect(), colorGroup() );
}

void ArticleListView::viewportPaintEvent(QPaintEvent *e)
{

    KListView::viewportPaintEvent(e);
    
    if(!e)
        return;
        
    QString message = QString::null;
    
    //kdDebug() << "visible articles: " << visibleArticles() << endl;
    
    if(childCount() != 0) // article list is not empty
    {
        if (visibleArticles() == 0)
        {
        message = i18n("<div align=center>"
                        "<h3>No matches</h3>"
                        "Filter does not match any articles, "
                        "please change your criteria and try again."
                        "</div>");
        }
        
    }
    else // article list is empty
    {
        if (!m_node) // no node selected
        {
            message = i18n("<div align=center>"
                       "<h3>No feed selected</h3>"
                       "This area is article list. "
                       "Select a feed from the feed list "
                       "and you will see its articles here."
                       "</div>");
        }
        else // empty node
        {
            // TODO: we could display message like "empty node, choose "fetch" to update it" 
        }
    }
    
    if (!message.isNull())
        paintInfoBox(message);
}

QDragObject *ArticleListView::dragObject()
{
    QDragObject *d = new QTextDrag(currentItem()->article().link().prettyURL(), this);
    return d;
}

void ArticleListView::slotPreviousArticle()
{
    QListViewItem *lvi = currentItem();
    
    if (!lvi && firstChild() )
    {
        setCurrentItem(firstChild());
        clearSelection();
        setSelected(firstChild(), true);
    }
    
    if (lvi && lvi->itemAbove())
    {
        setCurrentItem(lvi->itemAbove());
        clearSelection();
        setSelected( lvi->itemAbove(), true);
        ensureItemVisible(lvi->itemAbove());
    }
}

void ArticleListView::slotNextArticle()
{
    QListViewItem *lvi = currentItem();
    
    if (!lvi && firstChild() )
    {
        setCurrentItem(firstChild());
        clearSelection();
        setSelected(firstChild(), true);
        return;
    }
    if (lvi && lvi->itemBelow())
    {
        setCurrentItem(lvi->itemBelow());
        clearSelection();
        setSelected(lvi->itemBelow(), true);
        ensureItemVisible(lvi->itemBelow());
    }
}

void ArticleListView::slotNextUnreadArticle()
{
    ArticleItem *it= static_cast<ArticleItem*>(currentItem());
    if (!it)
        it = static_cast<ArticleItem*>(firstChild());

    for ( ; it; it = static_cast<ArticleItem*>(it->nextSibling()))
    {
        if (it->isVisible() && it->article().status() != Article::Read)
        {
            setCurrentItem(it);
            clearSelection();
            setSelected(it, true);
            ensureItemVisible(it);
            return;
        }
    }
    // only reached when there is no unread article after the selected one
    if (m_node->unread() > 0)
    {
        it = static_cast<ArticleItem*>(firstChild());
        for ( ; it; it = static_cast<ArticleItem*>(it->nextSibling()))
        {
            if (it->isVisible() && it->article().status() != Article::Read)
            {
                setCurrentItem(it);
                clearSelection();
                setSelected(it, true);
                ensureItemVisible(it);
                return;
            }
        }
    }
}

void ArticleListView::slotPreviousUnreadArticle()
{
    if ( !currentItem() )
        slotNextUnreadArticle(); 

    QListViewItemIterator it( currentItem() );
    
    for ( ; it.current(); --it )
    {
        ArticleItem* ali = static_cast<ArticleItem*> (it.current());
        if (!ali)
            break;
        if (ali->isVisible() && ali->article().status() != Article::Read)
        {
            setCurrentItem(ali);
            clearSelection();
            setSelected(ali, true);
            ensureItemVisible(ali);
            return;
        }
    }
    // only reached when there is no unread article before the selected one
    if (m_node->unread() > 0)
    {
        it = static_cast<ArticleItem*>(lastChild());

        for ( ; it.current(); --it )
        {
            ArticleItem* ali = static_cast<ArticleItem*> (it.current());
            if (ali->isVisible()  && ali->article().status() != Article::Read)
            {
                setCurrentItem(ali);
                clearSelection();
                setSelected(ali, true);
                ensureItemVisible(ali);
                return;
            }
        }
    }
}

void ArticleListView::keyPressEvent(QKeyEvent* e)
{
    e->ignore();
}

void ArticleListView::slotSelectionChanged()
{
    ArticleItem* ai = dynamic_cast<ArticleItem*> (currentItem());
    if (ai)
        emit signalArticleChosen( ai->article() );
}

void ArticleListView::slotCurrentChanged(QListViewItem*/* item*/)
{/*
    ArticleItem* ai = dynamic_cast<ArticleItem*> (item);
    if (ai)
        emit signalCurrentArticleChanged( ai->article() );*/
} 


void ArticleListView::slotDoubleClicked(QListViewItem* item, const QPoint& p, int i)
{
    emit signalDoubleClicked(static_cast<ArticleItem*>(item), p, i);
}

void ArticleListView::slotContextMenu(KListView* /*list*/, QListViewItem* /*item*/, const QPoint& p)
{
    QWidget* w = ActionManager::getInstance()->container("article_popup");
    QPopupMenu* popup = static_cast<QPopupMenu *>(w);
    if (popup)
        popup->exec(p);
}
        
ArticleListView::~ArticleListView()
{
    Settings::setTitleWidth(columnWidth(0));
    Settings::setFeedWidth(columnWidth(1) > 0 ? columnWidth(1) : m_feedWidth);
    Settings::setSortColumn(sortColumn());
    Settings::setSortAscending(sortOrder() == Ascending);
    Settings::writeConfig();
    delete m_columnLayoutVisitor;
}

QValueList<ArticleItem*> ArticleListView::selectedArticleItems(bool includeHiddenItems) const
{
    QPtrList<QListViewItem> items = selectedItems(includeHiddenItems);
    QValueList<ArticleItem*> ret;
    for (QListViewItem* i = items.first(); i; i = items.next() )
        ret.append(static_cast<ArticleItem*>(i));
    return ret;
}

} // namespace Akregator

#include "articlelistview.moc"
// vim: ts=4 sw=4 et
