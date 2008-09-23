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

#include <QStyle>
#include <QApplication>
#include <QIcon>
#include <QClipboard>
#include <QHash>
#include <QString>
#include <QToolButton>

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
#include <kmimetype.h>
#include <kio/global.h>

#include "actionmanager.h"
#include "akregatorconfig.h"
#include "frame.h"
#include "framemanager.h"
#include "kernel.h"
#include "openurlrequest.h"

#include <cassert>

namespace Akregator {

class TabWidget::Private
{
private:
    TabWidget* const q;

public:
    explicit Private( TabWidget * qq ) : q( qq ) {}

    TabWidget* parent;
    QHash<QWidget*, Frame*> frames;
    QHash<int, Frame*> framesById;
    int currentMaxLength;
    QWidget* currentItem;
    QToolButton* tabsClose;

    uint tabBarWidthForMaxChars(int maxLength);
    void setTitle(const QString &title , QWidget* sender);
    void updateTabBarVisibility();
    Frame* currentFrame();
};

void TabWidget::Private::updateTabBarVisibility()
{
    q->setTabBarHidden( q->count() <= 1 );
}

TabWidget::TabWidget(QWidget * parent)
    :KTabWidget(parent), d(new Private( this ) )
{
    d->parent = this;
    d->currentMaxLength = 30;
    d->currentItem = 0;
    setMinimumSize(250,150);
    setTabReorderingEnabled(false);
    connect( this, SIGNAL( currentChanged(int) ),
             this, SLOT( slotTabChanged(int) ) );
    connect(this, SIGNAL(closeRequest(QWidget*)),
            this, SLOT(slotCloseRequest(QWidget*)));
    setHoverCloseButton(Settings::closeButtonOnTabs());

    d->tabsClose = new QToolButton(this);
    d->tabsClose->setShortcut(QKeySequence("Ctrl+W"));
    connect( d->tabsClose, SIGNAL( clicked() ), this,
            SLOT( slotRemoveCurrentFrame() ) );

    d->tabsClose->setIcon( KIcon( "tab-close" ) );
    d->tabsClose->setEnabled( false );
    d->tabsClose->adjustSize();
    d->tabsClose->setToolTip( i18n("Close the current tab"));
    setCornerWidget( d->tabsClose, Qt::TopRightCorner );
    setTabBarHidden( true );
}

TabWidget::~TabWidget()
{
    delete d;
}

void TabWidget::slotSettingsChanged()
{
    if (hoverCloseButton() != Settings::closeButtonOnTabs())
        setHoverCloseButton(Settings::closeButtonOnTabs());
}

void TabWidget::slotNextTab()
{
    setCurrentIndex((currentIndex()+1) % count());
}

void TabWidget::slotPreviousTab()
{
    if (currentIndex() == 0)
        setCurrentIndex(count()-1);
    else
        setCurrentIndex(currentIndex()-1);
}
void TabWidget::slotSelectFrame(int frameId)
{
    Frame* frame = d->framesById[frameId];
    if (frame && frame != d->currentFrame())
    {
        setCurrentWidget(frame);
        if (frame->part() && frame->part()->widget())
        {
            frame->part()->widget()->setFocus();
        }
        else
        {
            frame->setFocus();
        }
    }
}

void TabWidget::slotAddFrame(Frame* frame)
{
    if (!frame)
        return;
    d->frames.insert(frame, frame);
    d->framesById[frame->id()] = frame;
    addTab(frame, frame->title());
    connect(frame, SIGNAL(signalTitleChanged(Akregator::Frame*,QString)),
            this, SLOT(slotSetTitle(Akregator::Frame*,QString)));
    connect(frame, SIGNAL(signalIconChanged(Akregator::Frame*,QIcon)),
            this, SLOT(slotSetIcon(Akregator::Frame*,QIcon)));

    if(frame->id() > 0) // MainFrame doesn't emit signalPartDestroyed signals, neither should it
        connect(frame, SIGNAL(signalPartDestroyed(int)), this, SLOT(slotRemoveFrame(int)));
    slotSetTitle(frame, frame->title());
}

Frame * TabWidget::Private::currentFrame()
{
    QWidget* w = q->currentWidget();
    assert( frames[w] );
    return w ? frames[w] : 0;
}

void TabWidget::slotTabChanged(int index)
{
    
    Frame* frame = d->frames[widget(index)];
    d->tabsClose->setEnabled(frame && frame->isRemovable());
    emit signalCurrentFrameChanged(frame ? frame->id() : -1);
}

void TabWidget::tabInserted( int )
{
    d->updateTabBarVisibility();
}


void TabWidget::tabRemoved( int )
{
    d->updateTabBarVisibility();
}

void TabWidget::slotRemoveCurrentFrame()
{
    Frame* const frame = d->currentFrame();
    if (frame)
        emit signalRemoveFrameRequest(frame->id());
}

void TabWidget::slotRemoveFrame(int frameId)
{
    if (!d->framesById.contains(frameId))
        return;
    Frame* f = d->framesById[frameId];
    d->frames.remove(f);
    d->framesById.remove(frameId);
    removeTab(indexOf(f));
    f->deleteLater(); // removeTab doesn't remove the widget, so let's do it ourselves
    if (d->currentFrame())
      d->setTitle( d->currentFrame()->title(), currentWidget() );
}

// copied wholesale from KonqFrameTabs
uint TabWidget::Private::tabBarWidthForMaxChars( int maxLength )
{
    int hframe, overlap;
    QStyleOption o;
    hframe = parent->tabBar()->style()->pixelMetric( QStyle::PM_TabBarTabHSpace, &o, parent );
    overlap = parent->tabBar()->style()->pixelMetric( QStyle::PM_TabBarTabOverlap, &o, parent );

    QFontMetrics fm = parent->tabBar()->fontMetrics();
    int x = 0;
    for (int i = 0; i < parent->count(); ++i)
    {
        Frame* f = frames[parent->widget(i)];
        QString newTitle = f->title();
        if ( newTitle.length() > maxLength )
            newTitle = newTitle.left( maxLength-3 ) + "...";

        int lw = fm.width( newTitle );
        int iw = parent->tabBar()->tabIcon( i ).pixmap( parent->tabBar()->style()->pixelMetric(
QStyle::PM_SmallIconSize ), QIcon::Normal
).width() + 4;

        x += ( parent->tabBar()->style()->sizeFromContents( QStyle::CT_TabBarTab, &o,
               QSize( qMax( lw + hframe + iw, QApplication::globalStrut().width() ), 0 ), parent ) ).width();
    }
    return x;
}

void TabWidget::slotSetTitle(Frame* frame, const QString& title)
{
    d->setTitle(title, frame);
}

void TabWidget::slotSetIcon(Akregator::Frame* frame, const QIcon& icon)
{
    const int idx = indexOf( frame );
    if ( idx < 0 )
        return;
    setTabIcon( idx, icon );
}

void TabWidget::Private::setTitle( const QString &title, QWidget* sender)
{
    int senderIndex = parent->indexOf(sender);

    parent->setTabToolTip( senderIndex, QString() );

    uint lcw=0, rcw=0;
    int tabBarHeight = parent->tabBar()->sizeHint().height();

    QWidget* leftCorner = parent->cornerWidget( Qt::TopLeftCorner );

    if ( leftCorner  && leftCorner->isVisible() )
        lcw = qMax( leftCorner->width(), tabBarHeight );

    QWidget* rightCorner = parent->cornerWidget( Qt::TopRightCorner );

    if ( rightCorner && rightCorner->isVisible() )
        rcw = qMax( rightCorner->width(), tabBarHeight );
    uint maxTabBarWidth = parent->width() - lcw - rcw;

    int newMaxLength = 30;

    for ( ; newMaxLength > 3; newMaxLength-- )
    {
        if ( tabBarWidthForMaxChars( newMaxLength ) < maxTabBarWidth )
            break;
    }

    QString newTitle = title;
    if ( newTitle.length() > newMaxLength )
    {
        parent->setTabToolTip( senderIndex, newTitle );
        newTitle = newTitle.left( newMaxLength-3 ) + "...";
    }

    newTitle.replace( '&', "&&" );

    if ( parent->tabText(senderIndex) != newTitle )
        parent->setTabText( senderIndex, newTitle );

    if( newMaxLength != currentMaxLength )
    {
        for( int i = 0; i < parent->count(); ++i)
        {
            Frame* f = frames[parent->widget(i)];
            newTitle = f->title();
            int index = parent->indexOf(parent->widget( i ));
            parent->setTabToolTip( index, QString() );

            if ( newTitle.length() > newMaxLength )
            {
                parent->setTabToolTip( index, newTitle );
                newTitle = newTitle.left( newMaxLength-3 ) + "...";
            }

            newTitle.replace( '&', "&&" );
            if ( newTitle != parent->tabText( index ) )
                parent->setTabText( index, newTitle );
        }
        currentMaxLength = newMaxLength;
    }
}

void TabWidget::contextMenu(int i, const QPoint &p)
{
    QWidget* w = ActionManager::getInstance()->container("tab_popup");
    d->currentItem = widget(i);
    //kDebug() << indexOf(d->currentItem);
    // FIXME: do not hardcode index of maintab
    if (w && indexOf(d->currentItem) != 0)
        static_cast<QMenu *>(w)->exec(p);
    d->currentItem = 0;
}

void TabWidget::slotDetachTab()
{
    if (!d->currentItem || indexOf(d->currentItem) == -1)
        d->currentItem = currentWidget();

    Frame* frame = d->frames[d->currentItem];

    if (frame && frame->url().isValid() && frame->isRemovable())
    {
        OpenUrlRequest request;
        request.setUrl(frame->url());
        request.setOptions(OpenUrlRequest::ExternalBrowser);
        emit signalOpenUrlRequest(request);
        slotCloseTab();
    }
}

void TabWidget::slotCopyLinkAddress()
{
    if(!d->currentItem || indexOf(d->currentItem) == -1)
        d->currentItem = currentWidget();
    Frame* frame = d->frames[d->currentItem];

    if (frame && frame->url().isValid())
    {
        KUrl url = frame->url();
        // don't set url to selection as it's a no-no according to a fd.o spec
        //kapp->clipboard()->setText(url.prettyUrl(), QClipboard::Selection);
        kapp->clipboard()->setText(url.prettyUrl(), QClipboard::Clipboard);
    }
}

void TabWidget::slotCloseTab()
{
    if (!d->currentItem || indexOf(d->currentItem) == -1)
        d->currentItem = currentWidget();
    if (d->frames[d->currentItem] == 0 || !d->frames[d->currentItem]->isRemovable() )
        return;

    emit signalRemoveFrameRequest(d->frames[d->currentItem]->id());
}

void TabWidget::initiateDrag(int tab)
{
    Frame* frame = d->frames[widget(tab)];

    if (frame && frame->url().isValid())
    {
        KUrl::List lst;
        lst.append( frame->url() );
        QDrag* drag = new QDrag( this );
        QMimeData *md = new QMimeData;
        drag->setMimeData( md );
        lst.populateMimeData( md );
        drag->setPixmap( KIO::pixmapForUrl( lst.first(), 0, KIconLoader::Small ) );
        drag->start();
    }
}

void TabWidget::slotCloseRequest(QWidget* widget)
{
    if (d->frames[widget])
        emit signalRemoveFrameRequest(d->frames[widget]->id());
}

} // namespace Akregator

#include "tabwidget.moc"
