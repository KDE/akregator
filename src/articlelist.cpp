/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "articlelist.h"
#include "feed.h"
#include "myarticle.h"
#include "treenode.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>

#include <qdatetime.h>
#include <qwhatsthis.h>
#include <qheader.h>

using namespace Akregator;
using namespace RSS;

struct ArticleListItem::Private
{
    MyArticle article;
    Feed *feed;
};

ArticleListItem::ArticleListItem( QListView *parent, QListViewItem *after,MyArticle a, Feed *feed)
    : KListViewItem( parent, after, KCharsets::resolveEntities(a.title()), KGlobal::locale()->formatDateTime(a.pubDate(), true, false) )
    , d(new Private)
{
    d->article = a;
    d->feed = feed;
	if (parent->columns() > 2)
	{
		setText(2, text(1));
		setText(1,feed->title());
	}
}

ArticleListItem::~ArticleListItem()
{
    delete d;
}

/*
int ArticleListItem::compare( QListViewItem *i, int col, bool ascending ) const
{
    ArticleListItem *item = static_cast<ArticleListItem *>(i);
    if (!item) return 0;

    if ( item->d->article.pubDate().isValid() && d->article.pubDate().isValid() )
    {
        int diff = d->article.pubDate().secsTo( item->d->article.pubDate() );
        return ascending ? diff : -diff;
    }

    return 0;
}*/

MyArticle ArticleListItem::article()
{
    return d->article;
}

Feed *ArticleListItem::feed()
{
    return d->feed;
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


/* ==================================================================================== */

ArticleList::ArticleList(QWidget *parent, const char *name)
    : KListView(parent, name), m_node(0), m_columnMode(feedMode)
{
    setMinimumSize(250, 150);
    addColumn(i18n("Article"));
    addColumn(i18n("Date"));
    setRootIsDecorated(false);
    setItemsRenameable(false);
    setItemsMovable(false);
    setAllColumnsShowFocus(true);
    setDragEnabled(false); // FIXME before we implement dragging between archived feeds??
    setAcceptDrops(false); // FIXME before we implement dragging between archived feeds??
    setFullWidth(false);
    setSorting(-1); // do not sort in the listview, Feed will take care of sorting
    setDragAutoScroll(true);
    setDropHighlighter(false);

    header()->setStretchEnabled(true, 0);

    QWhatsThis::add(this, i18n("<h2>Article list</h2>"
        "Here you can browse articles from the currently selected feed. "
        "You can also mark feeds as persistent (P column) "
        "or open article in another tab or even external browser window "
        "using right-click menu."));
}
void ArticleList::slotSetFilter(const ArticleFilter& textFilter, const ArticleFilter& statusFilter)
{
    if ( (textFilter != m_textFilter) || (statusFilter != m_statusFilter) )
    {
        m_textFilter = textFilter;
        m_statusFilter = statusFilter;
               
        slotUpdate();
    }
}

void ArticleList::slotShowNode(TreeNode* node)
{
    kdDebug() << "slotShowNode entered" << endl;
     if (!node)
        return slotClear();
     
    if (m_node)
    {
        disconnect(m_node, SIGNAL(signalChanged()), this, SLOT(slotUpdate()) );
        disconnect(m_node, SIGNAL(signalDestroyed()), this, SLOT(slotClear()) );
    }
    
    connect(node, SIGNAL(signalChanged()), this, SLOT(slotUpdate()) );
    connect(node, SIGNAL(signalDestroyed()), this, SLOT(slotClear()) );
    
    clear();
    
    if ( node->isGroup() && m_columnMode == feedMode )
    {
        addColumn(i18n("Date"));
        setColumnText(1,i18n("Feed"));
        setColumnWidthMode(2, QListView::Maximum);
        setColumnWidthMode(1, QListView::Manual);
        setColumnWidthMode(0, QListView::Manual);
        m_columnMode = groupMode;
    }
    else
    {
        if ( !node->isGroup() && m_columnMode == groupMode)
        {    
            setColumnText(1,i18n("Date"));
            int oldw=columnWidth(0)+columnWidth(1);
            removeColumn(2);
            setColumnWidthMode(1, QListView::Maximum);
            setColumnWidthMode(0, QListView::Manual);
            setColumnWidth(0, oldw); // resize title col to old title col + feed col width
            m_columnMode = feedMode;
        } 
    }
    m_node = node;
    
    slotUpdate();
        
    kdDebug() << "slotShowNode left" << endl;
}

void ArticleList::slotClear()
{
    kdDebug() << "slotClear entered" << endl;
    if (m_node)
    {
        disconnect(m_node, SIGNAL(signalChanged()), this, SLOT(slotUpdate()) );
        disconnect(m_node, SIGNAL(signalDestroyed()), this, SLOT(slotClear()) );
    }
    m_node = 0;
    
    clear();
    
    kdDebug() << "slotClear left" << endl;
}

void ArticleList::slotUpdate()
{
    kdDebug() << "slotUpdate entered" << endl;

    if (!m_node) 
        return;    
    
    setUpdatesEnabled(false);
    
    clear();
    
    ArticleSequence articles = m_node->articles();
    
    ArticleSequence::ConstIterator end = articles.end();
    ArticleSequence::ConstIterator it = articles.begin();
    
    for ( ; it != end; ++it)
    {
        // better use setVisible for filtering?   
         if ( m_textFilter.matches(*it) && m_statusFilter.matches(*it) )
             new ArticleListItem(this, lastChild(), *it, (*it).feed() );
    }        
    setUpdatesEnabled(true);
    triggerUpdate();

    kdDebug() << "slotUpdate left" << endl;
}
 
ArticleList::~ArticleList()
{}

#include "articlelist.moc"
