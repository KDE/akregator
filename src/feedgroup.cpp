/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/
#include "articlesequence.h" 
#include "feedgroup.h"
#include "feedscollection.h"
#include <qlistview.h>
#include <qdom.h>

#include <kdebug.h>

namespace Akregator {

FeedGroup::FeedGroup(QListViewItem *i, FeedsCollection *coll)
    : QObject(0, "some_random_name")
    , m_title()
    , m_item(i)
    , m_collection(coll)
{
}

FeedGroup::~FeedGroup()
{
    emit signalDestroyed();
}

void FeedGroup::destroy()
{
   if (m_collection)
    m_collection->remove(m_item);
   delete this;
}


void FeedGroup::setTitle(const QString &title)
{
    m_title = title;
    if (m_item)
        m_item->setText(0, title);
}

ArticleSequence FeedGroup::articles() const
{
    ArticleSequence seq;
    for (QListViewItem* i = m_item->firstChild(); i; i = i->nextSibling() )
    {
        FeedGroup* fg = static_cast<FeedGroup*> (m_collection->find(i));
        seq += fg->articles();
    }    
     return seq;
}

void FeedGroup::setItem(QListViewItem *i)
{
    m_item=i;
}
    
void FeedGroup::setCollection(FeedsCollection *c)
{
    m_collection=c;
}

QDomElement FeedGroup::toXml( QDomElement parent, QDomDocument document ) const
{
    QDomElement el = document.createElement( "outline" );
    el.setAttribute( "text", title() );
    parent.appendChild( el );
    return el;
}

};

#include "feedgroup.moc"
