/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

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
        "You can also mark feeds as persistent (P column) "
        "or open article in another tab or even external browser window "
        "using right-click menu."));

    connect(this, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)) );
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
                ali->setSelected(true);
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
        if ((it->article().status()==MyArticle::Unread) ||
             (it->article().status()==MyArticle::New) )
        {
            setSelected(it, true);
            ensureItemVisible(it);
            return;
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
