/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "articlelist.h"
#include "feed.h"
#include "myarticle.h"

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
        : KListView(parent, name)
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


ArticleList::~ArticleList()
{}

#include "articlelist.moc"
