/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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

#include "tabwidget.h"

#include <qstyle.h>
#include <qapplication.h>
#include <qicon.h>
#include <qclipboard.h>
#include <qmap.h>
#include <QHash>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QMenu>
#include <QStyleOption>

#include <kapplication.h>
#include <kdebug.h>
#include <ktabwidget.h>
#include <ktabbar.h>
#include <kmenu.h>
#include <krun.h>
#include <klocale.h>
#include <khtmlview.h>
#include <khtml_part.h>
#include <kiconloader.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <k3urldrag.h>
#include <kmimetype.h>
#include <ktoolinvocation.h>

#include "actionmanager.h"
#include "akregatorconfig.h"
#include "frame.h"
#include "framemanager.h"
#include "kernel.h"

namespace Akregator {

class TabWidget::TabWidgetPrivate
{
    public:
    QHash<QWidget*, Frame*> frames;
    int CurrentMaxLength;
    QWidget* currentItem;
    QToolButton* tabsClose;
};

TabWidget::TabWidget(QWidget * parent)
        :KTabWidget(parent), d(new TabWidgetPrivate)
{
    d->CurrentMaxLength = 30;
    setMinimumSize(250,150);
    setTabReorderingEnabled(false);
    connect( this, SIGNAL( currentChanged(QWidget *) ), this,
            SLOT( slotTabChanged(QWidget *) ) );
    connect(this, SIGNAL(closeRequest(QWidget*)), this, SLOT(slotCloseRequest(QWidget*)));
    setHoverCloseButton(Settings::closeButtonOnTabs());

    d->tabsClose = new QToolButton(this);
    d->tabsClose->setAccel(QKeySequence("Ctrl+W"));
    connect( d->tabsClose, SIGNAL( clicked() ), this,
            SLOT( slotRemoveCurrentFrame() ) );

    d->tabsClose->setIconSet( SmallIconSet( "tab_remove" ) );
    d->tabsClose->adjustSize();
    d->tabsClose->setToolTip( i18n("Close the current tab"));
    setCornerWidget( d->tabsClose, Qt::TopRightCorner );
}

TabWidget::~TabWidget()
{
    delete d;
    d = 0;
}

void TabWidget::slotSettingsChanged()
{
    if (hoverCloseButton() != Settings::closeButtonOnTabs())
        setHoverCloseButton(Settings::closeButtonOnTabs());
}

void TabWidget::slotNextTab()
{
    setCurrentPage((currentPageIndex()+1) % count());
}

void TabWidget::slotPreviousTab()
{
    if (currentPageIndex() == 0)
        setCurrentPage(count()-1);
    else
        setCurrentPage(currentPageIndex()-1);
}

void TabWidget::addFrame(Frame* frame)
{
    if (!frame)
        return;
    d->frames.insert(frame, frame);
    // TODO: don't let tabwidget insert frames to the manager
    Kernel::self()->frameManager()->addFrame(frame);
    addTab(frame, frame->title());
    connect(frame, SIGNAL(signalTitleChanged(Frame*, const QString& )), this, SLOT(slotSetTitle(Frame*, const QString& )));
    slotSetTitle(frame, frame->title());
}

Frame *TabWidget::currentFrame()
{
    QWidget* w = currentPage();

    return w ? d->frames[w] : 0;
}

void TabWidget::slotTabChanged(QWidget *w)
{
    Frame* frame = d->frames[w];
    d->tabsClose->setEnabled(frame && frame->isRemovable());
    emit currentFrameChanged(frame);
}

void TabWidget::slotRemoveCurrentFrame()
{
    removeFrame(currentFrame());
}

void TabWidget::removeFrame(Frame *f)
{
    d->frames.remove(f);
    slotPreviousTab();
    removeTab(indexOf(f));
    // TODO: don't let tabwidget insert frames to the manager
    Kernel::self()->frameManager()->removeFrame(f);
    delete f;
    setTitle( currentFrame()->title(), currentPage() );
}

// copied wholesale from KonqFrameTabs
uint TabWidget::tabBarWidthForMaxChars( int maxLength )
{
    int hframe, overlap;
    QStyleOption o;
    hframe = tabBar()->style()->pixelMetric( QStyle::PM_TabBarTabHSpace, &o, this );
    overlap = tabBar()->style()->pixelMetric( QStyle::PM_TabBarTabOverlap, &o, this );

    QFontMetrics fm = tabBar()->fontMetrics();
    int x = 0;
    for( int i=0; i < count(); ++i ) {
        Frame *f=d->frames[page(i)];
        QString newTitle=f->title();
        if ( newTitle.length() > maxLength )
            newTitle = newTitle.left( maxLength-3 ) + "...";

        int lw = fm.width( newTitle );
        int iw = tabBar()->tabIcon( i ).pixmap( QIcon::Small, QIcon::Normal ).width() + 4;

        x += ( tabBar()->style()->sizeFromContents( QStyle::CT_TabBarTab, &o,
               QSize( qMax( lw + hframe + iw, QApplication::globalStrut().width() ), 0 ), this ) ).width();
    }
    return x;
}

void TabWidget::slotSetTitle(Frame* frame, const QString& title)
{
    setTitle(title, frame);
}

void TabWidget::setTitle( const QString &title , QWidget* sender)
{
    removeTabToolTip( sender );

    uint lcw=0, rcw=0;
    int tabBarHeight = tabBar()->sizeHint().height();
    if ( cornerWidget( Qt::TopLeftCorner ) && cornerWidget( Qt::TopLeftCorner )->isVisible() )
        lcw = qMax( cornerWidget( Qt::TopLeftCorner )->width(), tabBarHeight );
    if ( cornerWidget( Qt::TopRightCorner ) && cornerWidget( Qt::TopRightCorner )->isVisible() )
        rcw = qMax( cornerWidget( Qt::TopRightCorner )->width(), tabBarHeight );
    uint maxTabBarWidth = width() - lcw - rcw;

    int newMaxLength=30;
    for ( ; newMaxLength > 3; newMaxLength-- )
{
        if ( tabBarWidthForMaxChars( newMaxLength ) < maxTabBarWidth )
            break;
    }
    QString newTitle = title;
    if ( newTitle.length() > newMaxLength )
    {
        setTabToolTip( sender, newTitle );
        newTitle = newTitle.left( newMaxLength-3 ) + "...";
    }

    newTitle.replace( '&', "&&" );
    if ( tabText(indexOf(sender)) != newTitle )
        setTabText( indexOf(sender), newTitle );

    if( newMaxLength != d->CurrentMaxLength )
    {
        for( int i = 0; i < count(); ++i)
        {
            Frame *f=d->frames[page(i)];
            newTitle=f->title();
            removeTabToolTip( page( i ) );
            if ( newTitle.length() > newMaxLength )
            {
                setTabToolTip( page( i ), newTitle );
                newTitle = newTitle.left( newMaxLength-3 ) + "...";
            }

            newTitle.replace( '&', "&&" );
            if ( newTitle != tabText( indexOf( page( i ) ) ) )
                setTabText( indexOf( page( i ) ), newTitle );
        }
        d->CurrentMaxLength = newMaxLength;
    }
}

void TabWidget::contextMenu(int i, const QPoint &p)
{
    QWidget* w = ActionManager::getInstance()->container("tab_popup");
    d->currentItem = page(i);
    //kDebug() << indexOf(d->currentItem) << endl;
    // FIXME: do not hardcode index of maintab
    if (w && indexOf(d->currentItem) != 0)
        static_cast<QMenu *>(w)->exec(p);
    d->currentItem = 0;
}

void TabWidget::slotDetachTab()
{
    if (!d->currentItem || indexOf(d->currentItem) == -1)
        d->currentItem = currentPage();

    Frame* frame = d->frames[d->currentItem];

    if (frame && frame->url().isValid() && frame->isRemovable())
    {
        KToolInvocation::invokeBrowser(frame->url().url(), "0");
        slotCloseTab();
    }
}

void TabWidget::slotCopyLinkAddress()
{
    if(!d->currentItem || indexOf(d->currentItem) == -1)
        d->currentItem = currentPage();
    Frame* frame = d->frames[d->currentItem];

    if (frame && frame->url().isValid())
    {
        KUrl url = frame->url();
        kapp->clipboard()->setText(url.prettyURL(), QClipboard::Selection);
        kapp->clipboard()->setText(url.prettyURL(), QClipboard::Clipboard);
    }
}

void TabWidget::slotCloseTab()
{
    if (!d->currentItem || indexOf(d->currentItem) == -1)
        d->currentItem = currentPage();
    if (d->frames[d->currentItem] == 0 || !d->frames[d->currentItem]->isRemovable() )
        return;

    //kDebug() << "index: " << indexOf(d->currentItem) << endl;
    removeFrame(d->frames[d->currentItem]);
    //kDebug() << "index: " << indexOf(d->currentItem) << endl;

    /* no need to do this? looks like it points to main page after removeFrame...
    delete d->currentItem;
    d->currentItem = 0;*/

}

void TabWidget::initiateDrag(int tab)
{
    Frame* frame = d->frames[page(tab)];

    if (frame && frame->url().isValid())
    {
        KUrl::List lst;
        lst.append( frame->url() );
        K3URLDrag* drag = new K3URLDrag( lst, this );
        drag->setPixmap( KMimeType::pixmapForURL( lst.first(), 0, KIcon::Small ) );
        drag->dragCopy();
    }
}

void TabWidget::slotCloseRequest(QWidget* widget)
{
    if (d->frames[widget])
        removeFrame(d->frames[widget]);
}
} // namespace Akregator

#include "tabwidget.moc"
