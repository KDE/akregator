/***************************************************************************
 *     Copyright (C) 2004 by Sashmit Bhaduri                               *
 *     smt@vfemail.net                                                     *
 *                                                                         *
 *     Licensed under GPL.                                                 *
 ***************************************************************************/

#include "tabwidget.h"

#include <qstyle.h>
#include <qapplication.h>
#include <qiconset.h>
#include <qclipboard.h>

#include <kapplication.h>
#include <kdebug.h>
#include <ktabwidget.h>
#include <ktabbar.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <klocale.h>
#include <khtmlview.h>
#include <khtml_part.h>
#include <kiconloader.h>
#include <kurl.h>

#include "akregatorconfig.h"

using namespace Akregator;

TabWidget::TabWidget(QWidget * parent, const char *name)
        :KTabWidget(parent, name), m_CurrentMaxLength(30)
{
    setTabReorderingEnabled(false);
    connect( this, SIGNAL( currentChanged(QWidget *) ), this,
            SLOT( slotTabChanged(QWidget *) ) );
    connect(this, SIGNAL(closeRequest(QWidget*)), this, SLOT(slotCloseRequest(QWidget*)));
    setHoverCloseButton(Settings::closeButtonOnTabs());
}

TabWidget::~TabWidget()
{
}

void TabWidget::slotSettingsChanged()
{
    if (hoverCloseButton() != Settings::closeButtonOnTabs())
        setHoverCloseButton(Settings::closeButtonOnTabs());
}

void TabWidget::addFrame(Frame *f)
{
    if (!f || !f->widget()) return;
    m_frames.insert(f->widget(), f);
    addTab(f->widget(), f->title());
}

Frame *TabWidget::currentFrame()
{
    QWidget *w=currentPage();
    if (!w) return 0;
    return m_frames[w];
}

void TabWidget::slotTabChanged(QWidget *w)
{
    emit currentFrameChanged(m_frames[w]);
}

void TabWidget::removeFrame(Frame *f)
{
    f->setCompleted();
    m_frames.remove(f->widget());
    removePage(f->widget());
    delete f;
    setTitle( currentFrame()->title(), currentPage() );
}

// copied wholesale from KonqFrameTabs
unsigned int TabWidget::tabBarWidthForMaxChars( uint maxLength )
{
    int hframe, overlap;
    hframe    = tabBar()->style().pixelMetric( QStyle::PM_TabBarTabHSpace, this );
    overlap = tabBar()->style().pixelMetric( QStyle::PM_TabBarTabOverlap, this );

    QFontMetrics fm = tabBar()->fontMetrics();
    int x = 0;
    for( int i=0; i < count(); ++i ) {
        Frame *f=m_frames[page(i)];
        QString newTitle=f->title();
        if ( newTitle.length() > maxLength )
            newTitle = newTitle.left( maxLength-3 ) + "...";

        QTab* tab = tabBar()->tabAt( i );
        int lw = fm.width( newTitle );
        int iw = 0;
        if ( tab->iconSet() )
            iw = tab->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 4;

        x += ( tabBar()->style().sizeFromContents( QStyle::CT_TabBarTab, this,                             QSize( QMAX( lw + hframe + iw, QApplication::globalStrut().width() ), 0 ), QStyleOption( tab ) ) ).width();
    }
    return x;
}

void TabWidget::setTitle( const QString &title , QWidget* sender)
{
    removeTabToolTip( sender );

    Frame *f=m_frames[sender];
    if (f)
        f->setTitle(title);

    uint lcw=0, rcw=0;
    int tabBarHeight = tabBar()->sizeHint().height();
    if ( cornerWidget( TopLeft ) && cornerWidget( TopLeft )->isVisible() )
        lcw = QMAX( cornerWidget( TopLeft )->width(), tabBarHeight );
    if ( cornerWidget( TopRight ) && cornerWidget( TopRight )->isVisible() )
        rcw = QMAX( cornerWidget( TopRight )->width(), tabBarHeight );
    uint maxTabBarWidth = width() - lcw - rcw;

    uint newMaxLength=30;
    for ( ; newMaxLength > 3; newMaxLength-- ) {
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

    if( newMaxLength != m_CurrentMaxLength )
    {
        for( int i = 0; i < count(); ++i)
        {
            Frame *f=m_frames[page(i)];
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
        m_CurrentMaxLength = newMaxLength;
    }
}

void TabWidget::contextMenu(int i, const QPoint &p)
{
   currentItem = page(i);
   KPopupMenu popup;
   //popup.insertTitle(tabLabel(currentItem));
   int detachTab = popup.insertItem( SmallIcon("tab_breakoff"), i18n("Detach Tab"), this, SLOT( slotDetachTab() ) );
   int copyLink = popup.insertItem( i18n("Copy Link Address"), this, SLOT( slotCopyLinkAddress() ) );
   //popup.insertSeparator();
   int closeTab = popup.insertItem( SmallIcon("tab_remove"), i18n("Close Tab"), this, SLOT( slotCloseTab() ) );
   if(indexOf(currentItem) == 0) { // you can't detach or close articles tab..
      popup.setItemEnabled(detachTab, false);
      popup.setItemEnabled(closeTab, false);
      popup.setItemEnabled(copyLink, false);
   }
   popup.exec(p);
}

void TabWidget::slotDetachTab()
{
   if(!currentItem) return;
   KURL url;
   if (KHTMLView *view = dynamic_cast<KHTMLView*>(currentItem)) url = view->part()->url();
   else return;
   kapp->invokeBrowser(url.url(), "0");
   if (m_frames.find(currentItem) != NULL)
       removeFrame(m_frames.find(currentItem));
   delete currentItem;
   currentItem = 0;
}

void TabWidget::slotCopyLinkAddress()
{
    if(!currentItem) return;
    KURL url;
    if (KHTMLView *view = dynamic_cast<KHTMLView*>(currentItem)) url = view->part()->url();
    else return;
    QClipboard *cb = QApplication::clipboard();
    if(cb) cb->setText(url.prettyURL());
}

void TabWidget::slotCloseTab()
{
   if(!currentItem) return;
   if (m_frames.find(currentItem) != NULL)
       removeFrame(m_frames.find(currentItem));
   delete currentItem;
   currentItem = 0;
}

void TabWidget::slotCloseRequest(QWidget* widget)
{
    if (m_frames.find(widget) != NULL)
        removeFrame(m_frames.find(widget));
}
#include "tabwidget.moc"


// vim: set et ts=4 sts=4 sw=4:
