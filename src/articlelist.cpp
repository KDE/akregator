/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "articlelist.h"
#include "feed.h"

#include <klocale.h>
#include <kcharsets.h>

#include <qdatetime.h>
#include <qwhatsthis.h>
#include <qheader.h>

using namespace Akregator;
using namespace RSS;

struct ArticleListItem::Private
{
    Article article;
    Feed *feed;
};

ArticleListItem::ArticleListItem( QListView *parent, Article a, Feed *feed )
    : KListViewItem( parent, parent->lastItem(), KCharsets::resolveEntities(a.title()) )
    /* FIXME lastItem() is not needed because we will sort after adding */
{
    d = new Private;
    d->article = a;
    d->feed = feed;
}

int ArticleListItem::compare( QListViewItem *i, int col, bool ascending ) const
{
    ArticleListItem *item = static_cast<ArticleListItem *>(i);

    if ( i && item->d->article.pubDate().isValid() && d->article.pubDate().isValid() )
    {
        int diff = d->article.pubDate().secsTo( item->d->article.pubDate() );
        return ascending ? diff : -diff;
    }

    return 0;
}

Article ArticleListItem::article()
{
    return d->article;
}

/* ==================================================================================== */

ArticleList::ArticleList(QWidget *parent, const char *name)
        : KListView(parent, name)
{
    setMinimumSize(250, 150);
    addColumn(i18n("Articles"));
    addColumn(i18n("P"), 16);
    setRootIsDecorated(false);
    setItemsRenameable(false);
    setItemsMovable(false);
    setAllColumnsShowFocus(true);
    setDragEnabled(false); // FIXME before we implement dragging between archived feeds??
    setAcceptDrops(false); // FIXME before we implement dragging between archived feeds??
    setFullWidth(false);
    setSorting(0, false); // sort by date, descending
    setDragAutoScroll(true);
    setDropHighlighter(false);

    header()->setStretchEnabled(true, 0);

    QWhatsThis::add(this, i18n("<h2>Articles list</h2>"
        "Here you can browse articles from currently selected feed. "
        "You can also mark feeds as persistent (P column) "
        "or open article in another tab or even external browser window "
        "using right-click menu."));
}


ArticleList::~ArticleList()
{}

#include "articlelist.moc"
