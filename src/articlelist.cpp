/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "articlelist.h"

#include <klocale.h>

#include <qwhatsthis.h>
#include <qheader.h>

using namespace Akregator;

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
    setSorting(-1);
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
