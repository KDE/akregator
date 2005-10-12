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
#include <q3ptrdict.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <Q3PopupMenu>
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
#include "frame.h"
#include "akregatorconfig.h"

namespace Akregator {

class TabWidget::TabWidgetPrivate
{
    public:
    Q3PtrDict<Frame> frames;
    int CurrentMaxLength;
    QWidget* currentItem;
    QToolButton* tabsClose;
};

TabWidget::TabWidget(QWidget * parent, const char *name)
        :KTabWidget(parent, name), d(new TabWidgetPrivate)
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
    QToolTip::add(d->tabsClose, i18n("Close the current tab"));
    setCornerWidget( d->tabsClose, Qt::TopRight );
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

void TabWidget::addFrame(Frame *f)
{
    if (!f || !f->widget()) 
        return;
    d->frames.insert(f->widget(), f);
    addTab(f->widget(), f->title());
    connect(f, SIGNAL(titleChanged(Frame*, const QString& )), this, SLOT(slotSetTitle(Frame*, const QString& )));
    slotSetTitle(f, f->title());
}

Frame *TabWidget::currentFrame()
{
    QWidget* w = currentPage();
    
    return w ? d->frames[w] : 0;
}

void TabWidget::slotTabChanged(QWidget *w)
{
    // FIXME: Don't hardcode the tab position of main frame
    d->tabsClose->setDisabled(currentPageIndex() == 0);
    emit currentFrameChanged(d->frames[w]);
}

void TabWidget::slotRemoveCurrentFrame()
{
    removeFrame(currentFrame());
}

void TabWidget::removeFrame(Frame *f)
{
    f->setCompleted();
    d->frames.remove(f->widget());
    removePage(f->widget());
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
    setTitle(title, frame->widget());
}

void TabWidget::setTitle( const QString &title , QWidget* sender)
{
    removeTabToolTip( sender );
   
    uint lcw=0, rcw=0;
    int tabBarHeight = tabBar()->sizeHint().height();
    if ( cornerWidget( Qt::TopLeft ) && cornerWidget( Qt::TopLeft )->isVisible() )
        lcw = qMax( cornerWidget( Qt::TopLeft )->width(), tabBarHeight );
    if ( cornerWidget( Qt::TopRight ) && cornerWidget( Qt::TopRight )->isVisible() )
        rcw = qMax( cornerWidget( Qt::TopRight )->width(), tabBarHeight );
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
    if ( tabLabel( sender ) != newTitle )
        changeTab( sender, newTitle );

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
            if ( newTitle != tabLabel( page( i ) ) )
                    changeTab( page( i ), newTitle );
        }
        d->CurrentMaxLength = newMaxLength;
    }
}

void TabWidget::contextMenu(int i, const QPoint &p)
{
    QWidget* w = ActionManager::getInstance()->container("tab_popup");
    d->currentItem = page(i);
    //kdDebug() << indexOf(d->currentItem) << endl;
    if (w && indexOf(d->currentItem) != 0)
        static_cast<Q3PopupMenu *>(w)->exec(p);
    d->currentItem = 0;
}

void TabWidget::slotDetachTab()
{
    if (!d->currentItem || indexOf(d->currentItem) == -1) 
        d->currentItem = currentPage();

    if (indexOf(d->currentItem) == 0) 
        return;

    KURL url;
    KHTMLView* view = dynamic_cast<KHTMLView*>(d->currentItem);
    
    if (!view)
        return;

    url = view->part()->url();

    KToolInvocation::invokeBrowser(url.url(), "0");
    slotCloseTab();
}

void TabWidget::slotCopyLinkAddress()
{
    if(!d->currentItem || indexOf(d->currentItem) == -1) 
        d->currentItem = currentPage();
    if(indexOf(d->currentItem) == 0) 
        return;

    KURL url;
    KHTMLView* view = dynamic_cast<KHTMLView*>(d->currentItem);
    
    if (!view)
        return;

    url = view->part()->url();
    
    kapp->clipboard()->setText(url.prettyURL(), QClipboard::Selection);
    kapp->clipboard()->setText(url.prettyURL(), QClipboard::Clipboard);
}

void TabWidget::slotCloseTab()
{
    if (!d->currentItem || indexOf(d->currentItem) == -1) 
        d->currentItem = currentPage();
    if (indexOf(d->currentItem) == 0) 
        return;
    if (d->frames.find(d->currentItem) != NULL)
        removeFrame(d->frames.find(d->currentItem));
    delete d->currentItem;
    d->currentItem = 0;
}

void TabWidget::initiateDrag(int tab)
{
    if (tab == 0) // don't initiate drag for the main tab
        return;
        
    Frame* frame = d->frames[page(tab)];
  
    if (frame != 0)
    {
        KURL::List lst;
        lst.append( frame->part()->url() );
        K3URLDrag* drag = new K3URLDrag( lst, this );
        drag->setPixmap( KMimeType::pixmapForURL( lst.first(), 0, KIcon::Small ) );
        drag->dragCopy();
    }
}

void TabWidget::slotCloseRequest(QWidget* widget)
{
    if (d->frames.find(widget) != NULL)
        removeFrame(d->frames.find(widget));
}
} // namespace Akregator

#include "tabwidget.moc"
