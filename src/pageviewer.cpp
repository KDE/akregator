/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include "akregatorconfig.h"
#include "akregator_run.h" 
#include "feediconmanager.h"
#include "pageviewer.h"
#include "viewer.h"

#include <kaction.h>
#include <kapplication.h>
#include <kbookmark.h>
#include <kbookmarkmanager.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <khtml_settings.h>
#include <khtmlview.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kstdaccel.h>
#include <kparts/browserinterface.h>

#include <qclipboard.h>
#include <qcstring.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qmetaobject.h>
#include <qscrollview.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <private/qucomextra_p.h>

#include <cstdlib>
using std::abs;

namespace Akregator {


// taken from KDevelop
class PageViewer::HistoryEntry
{
    public:

    KURL url;
    QString title;
    QByteArray state;
    int id;

    HistoryEntry() {}
    HistoryEntry(const KURL& u, const QString& t=QString::null): url(u), title(t)   
    {
        id = abs( QTime::currentTime().msecsTo( QTime() ) );    // nasty, but should provide a reasonably unique number
    }
    
};

class PageViewer::PageViewerPrivate
{
    public:

    QValueList<HistoryEntry> history;
    QValueList<HistoryEntry>::Iterator current;
    
    KToolBarPopupAction* backAction;
    KToolBarPopupAction* forwardAction;
    KAction* reloadAction;
    KAction* stopAction;
    
    QString caption;
};           

 
PageViewer::PageViewer(QWidget *parent, const char *name)
    : Viewer(parent, name), d(new PageViewerPrivate)
{
    // this hack is necessary since the part looks for []HTML Settings] in
    // KGlobal::config() by default, which is wrong when running in Kontact
    KHTMLSettings* s = const_cast<KHTMLSettings*> (settings());
    s->init(Settings::self()->config());
    
    setXMLFile(locate("data", "akregator/pageviewer.rc"), true);

    QPair< KGuiItem, KGuiItem > backForward = KStdGuiItem::backAndForward();

    d->backAction = new KToolBarPopupAction(backForward.first, 
                                KStdAccel::shortcut(KStdAccel::Back), this, 
                                SLOT(slotBack()), actionCollection(), 
                                "pageviewer_back");

    connect(d->backAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotBackAboutToShow()));
    connect(d->backAction->popupMenu(), SIGNAL(activated(int)),
            this, SLOT(slotPopupActivated(int)));

    
    d->forwardAction = new KToolBarPopupAction(backForward.second, 
                                KStdAccel::shortcut(KStdAccel::Forward),this, 
                                SLOT(slotForward()), actionCollection(), 
                                "pageviewer_forward");

    connect(d->forwardAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotForwardAboutToShow()));
    connect(d->forwardAction->popupMenu(), SIGNAL(activated(int)),
            this, SLOT(slotPopupActivated(int)));

    d->reloadAction = new KAction(i18n("Reload"), "reload", 0,
                            this, SLOT(slotReload()),
                            actionCollection(), "pageviewer_reload");
    d->stopAction = new KAction(KStdGuiItem::guiItem(KStdGuiItem::Stop), 0,
                                 this, SLOT(slotStop()),
                                 actionCollection(), "pageviewer_stop");
 
    //connect( this, SIGNAL(popupMenu(const QString &, const QPoint &)), this, SLOT(slotPopupMenu(const QString &, const QPoint &)));

    d->backAction->setEnabled(false);
    d->forwardAction->setEnabled(false);
    d->stopAction->setEnabled(false);
    
    connect( this, SIGNAL(setWindowCaption (const QString &)),
            this, SLOT(slotSetCaption (const QString &)) );

    connect(this, SIGNAL(started(KIO::Job *)), this, SLOT(slotStarted(KIO::Job* )));
    connect(this, SIGNAL(completed()), this, SLOT(slotCompleted()));
    connect(this, SIGNAL(canceled(const QString &)), this, SLOT(slotCancelled(const QString &)));

    d->current = d->history.end();

    // uncomment this to load konq plugins (doesn't work properly and clutters the GUI)
    //loadPlugins( partObject(), this, instance() );
    
}

PageViewer::~PageViewer()
{
    delete d;
    d = 0;
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotBack()
{
    if ( d->current != d->history.begin() )
    {
        QValueList<HistoryEntry>::Iterator tmp = d->current;
        --tmp;
        restoreHistoryEntry(tmp);
    }
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotForward()
{
    if (  d->current != d->history.fromLast() )
    {
        QValueList<HistoryEntry>::Iterator tmp = d->current;
        ++tmp;
        restoreHistoryEntry(tmp);
    }
}

void PageViewer::slotBackAboutToShow()
{
    KPopupMenu *popup = d->backAction->popupMenu();
    popup->clear();

    if ( d->current == d->history.begin() )
        return;

    QValueList<HistoryEntry>::Iterator it = d->current;
    --it;
    
    int i = 0;
    while( i < 10 )
    {
        if ( it == d->history.begin() )
        {
            popup->insertItem( (*it).title, (*it).id );
            return;
        }
        
        popup->insertItem( (*it).title, (*it).id );
        ++i;
        --it;
    }
}

void PageViewer::slotForwardAboutToShow()
{
    KPopupMenu *popup = d->forwardAction->popupMenu();
    popup->clear();

    if ( d->current == d->history.fromLast() )
        return;

    QValueList<HistoryEntry>::Iterator it = d->current;
    ++it;
    
    int i = 0;
    while( i < 10 )
    {
        if ( it == d->history.fromLast() )
        {
            popup->insertItem( (*it).title, (*it).id );
            return;
        }
        
        popup->insertItem( (*it).title, (*it).id );
        ++i;
        ++it;
    }
}


void PageViewer::slotReload()
{
    openURL( url() );
}

void PageViewer::slotStop()
{
    closeURL();
}

bool PageViewer::openURL(const KURL& url)
{
    updateHistoryEntry(); // update old history entry before switching to the new one
    emit started(0);

    bool val = KHTMLPart::openURL(url);
    
    addHistoryEntry(url); // add new URL to history
    
    d->backAction->setEnabled( d->current != d->history.begin() );
    d->forwardAction->setEnabled( d->current != d->history.fromLast() );
  
    QString favicon = FeedIconManager::self()->iconLocation(url);
    if (!favicon.isEmpty()) 
        emit setTabIcon(QPixmap(KGlobal::dirs()->findResource("cache", favicon+".png")));
    else
        emit setTabIcon(SmallIcon("html"));

    return val;
}


void PageViewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
    updateHistoryEntry();
    if (args.doPost())
    {
        browserExtension()->setURLArgs(args);
        openURL(url);
    }

}

void PageViewer::slotPopupActivated( int id )
{
    QValueList<HistoryEntry>::Iterator it = d->history.begin();
    while( it != d->history.end() )
    {
        if ( (*it).id == id )
        {
            restoreHistoryEntry(it);
            return;
        }
        ++it;
    }
}

void PageViewer::updateHistoryEntry()
{
    (*d->current).title = d->caption;
    (*d->current).state = QByteArray(); // Start with empty buffer.
    QDataStream stream( (*d->current).state, IO_WriteOnly);
    browserExtension()->saveState(stream);
}

void PageViewer::restoreHistoryEntry(const QValueList<HistoryEntry>::Iterator& entry)
{
    updateHistoryEntry();
    
    QDataStream stream( (*entry).state, IO_ReadOnly );
    browserExtension()->restoreState( stream );
    d->current = entry;
    d->backAction->setEnabled( d->current != d->history.begin() );
    d->forwardAction->setEnabled( d->current != d->history.fromLast() );
    //openURL( entry.url ); // TODO read state
    

}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::addHistoryEntry(const KURL& url)
{
    QValueList<HistoryEntry>::Iterator it = d->current;
    
    // if We're not already the last entry, we truncate the list here before adding an entry
    if ( it != d->history.end() && it != d->history.fromLast() )
    {
        d->history.erase( ++it, d->history.end() );
    }
    HistoryEntry newEntry( url, url.url() );

    // Only save the new entry if it is different from the last
    if ( newEntry.url != (*d->current).url )
    {
        d->history.append( newEntry );
        d->current = d->history.fromLast();
    }
    updateHistoryEntry();
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotStarted( KIO::Job * )
{
    d->stopAction->setEnabled(true);
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotCompleted( )
{
    d->stopAction->setEnabled(false);
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotCancelled( const QString & /*errMsg*/ )
{
    d->stopAction->setEnabled(false);
}

void PageViewer::urlSelected(const QString &url, int button, int state, const QString &_target, KParts::URLArgs args)
{
    if (button == LeftButton)
    {
        m_url = completeURL(url);
        browserExtension()->setURLArgs(args); 
        slotOpenLinkInThisTab();
    }
    else
    {
        Viewer::urlSelected(url,button,state,_target,args);
    }
}

void PageViewer::slotSetCaption(const QString& cap) 
{
    d->caption = cap;
    (*d->current).title = cap;
}

void PageViewer::slotPaletteOrFontChanged()
{
    kdDebug() << "PageViewer::slotPaletteOrFontChanged()" << endl;
    // taken from KonqView (kdebase/konqueror/konq_view.cc)
    
    QObject *obj = KParts::BrowserExtension::childObject(this);
    if ( !obj ) // not all views have a browser extension !
        return;
    
    int id = obj->metaObject()->findSlot("reparseConfiguration()");
    if (id == -1)
        return;
    QUObject o[1];

    obj->qt_invoke(id, o);
    
    // this hack is necessary since the part looks for []HTML Settings] in
    // KGlobal::config() by default, which is wrong when running in Kontact
    // NOTE: when running in Kontact, immediate updating doesn't work
    KHTMLSettings* s = const_cast<KHTMLSettings*> (settings());
    s->init(Settings::self()->config());
}

void PageViewer::slotGlobalBookmarkArticle()
{
    KBookmarkManager *mgr = KBookmarkManager::userBookmarksManager();
    KBookmarkGroup grp = mgr->root();
    grp.addBookmark(mgr, d->caption, toplevelURL());
    mgr->emitChanged(grp);
    mgr->save();
}


void PageViewer::slotPopupMenu(KXMLGUIClient*, const QPoint& p, const KURL& kurl, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags kpf, mode_t)
{
    m_url = kurl;
    QString url = kurl.url(); // maximal url confusion
    
    const bool showReload = (kpf & KParts::BrowserExtension::ShowReload) != 0;
    const bool showNavigationItems = (kpf & KParts::BrowserExtension::ShowNavigationItems) != 0;
    const bool isLink = (kpf & (KParts::BrowserExtension::ShowNavigationItems | KParts::BrowserExtension::ShowTextSelectionItems)) == 0;
    const bool isSelection = (kpf & KParts::BrowserExtension::ShowTextSelectionItems) != 0;
        
    KPopupMenu popup(this->widget());

    int idNewWindow = -2;
    if (isLink)
    {
        idNewWindow = popup.insertItem(SmallIcon("tab_new"),i18n("Open Link in New &Tab"), this, SLOT(slotOpenLinkInForegroundTab()));
        popup.setWhatsThis(idNewWindow, i18n("<b>Open Link in New Tab</b><p>Opens current link in a new tab."));
        popup.insertItem(SmallIcon("window_new"), i18n("Open Link in External &Browser"), this, SLOT(slotOpenLinkInBrowser()));
                
        popup.insertSeparator();
        action("savelinkas")->plug(&popup);    
        KAction* copylinkaddress = action("copylinkaddress");
        if (copylinkaddress)
        {
            copylinkaddress->plug( &popup);
            //popup.insertSeparator();
        }
    }
    else // we are not on a link
    {
        if (showNavigationItems)
        {
            d->backAction->plug( &popup );
            d->forwardAction->plug( &popup );
        }

        if (showReload)
            d->reloadAction->plug(&popup);
        
        d->stopAction->plug(&popup);
        
        popup.insertSeparator();
        
        if (isSelection)
        {
            action("viewer_copy")->plug(&popup);
            popup.insertSeparator();
        }

        KAction* incFontAction = this->action("incFontSizes");
        KAction* decFontAction = this->action("decFontSizes");
        if ( incFontAction && decFontAction )
        {
            incFontAction->plug( &popup );
            decFontAction->plug( &popup );
            popup.insertSeparator();
        }
    
        popup.insertItem(SmallIcon("window_new"), i18n("Open Page in External Browser"), this, SLOT(slotOpenLinkInBrowser()));
    
        action("viewer_print")->plug(&popup);
        popup.insertSeparator();
        
        KAction *ac = action("setEncoding");
        if (ac)
            ac->plug(&popup);
        popup.insertItem(SmallIcon("bookmark_add"),i18n("Add to Konqueror Bookmarks"), this, SLOT(slotGlobalBookmarkArticle()));
    }
    
    int r = popup.exec(p);
    
    if (r == idNewWindow)
    {
        KURL kurl;
        if (!KURL(url).path().startsWith("/"))
        {
            kdDebug() << "processing relative url: " << url << endl;
            if (url.startsWith("#"))
            {
                kurl = KURL(PageViewer::url());
                kurl.setRef(url.mid(1));
            }
            else
                kurl = KURL(PageViewer::url().upURL().url(true)+url);
        }
        else
            kurl = KURL(url);
//    kurl.addPath(url);
        if (kurl.isValid()) {
            //slotOpenInNewWindow(kurl);
        }
//      ( kurl );
    }
}

} // namespace Akregator 

#include "pageviewer.moc"
