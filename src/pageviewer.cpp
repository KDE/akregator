/***************************************************************************
 *   Copyright (C) 2004 by Sashmit Bhaduri                                 *
 *   smt@vfemail.net                                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

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
#include <kparts/browserinterface.h>

#include <qclipboard.h>
#include <qdatetime.h>
#include <qfile.h>
#include <qvaluelist.h>
#include <qscrollview.h>

#include <qmetaobject.h>
#include <private/qucomextra_p.h>

using namespace Akregator;

PageViewer::PageViewer(QWidget *parent, const char *name)
    : Viewer(parent, name)
{
    // this hack is necessary since the part looks for []HTML Settings] in
    // KGlobal::config() by default, which is wrong when running in Kontact
    KHTMLSettings* s = const_cast<KHTMLSettings*> (settings());
    s->init(Settings::self()->config());
    
    setXMLFile(locate("data", "akregator/pageviewer.rc"), true);
    
    m_backAction = new KToolBarPopupAction(i18n("Back"), "back", 0, this, SLOT(slotBack()), actionCollection(), "pageviewer_back");

    connect(m_backAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotBackAboutToShow()));
    connect(m_backAction->popupMenu(), SIGNAL(activated(int)),
            this, SLOT(slotPopupActivated(int)));

    
    m_forwardAction = new KToolBarPopupAction(i18n("Forward"), "forward", 0, this, SLOT(slotForward()), actionCollection(), "pageviewer_forward");

    connect(m_forwardAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotForwardAboutToShow()));
    connect(m_forwardAction->popupMenu(), SIGNAL(activated(int)),
            this, SLOT(slotPopupActivated(int)));

    m_reloadAction = new KAction(i18n("Reload"), "reload", 0,
                            this, SLOT(slotReload()),
                            actionCollection(), "pageviewer_reload");
    m_stopAction = new KAction(i18n("Stop"), "stop", 0,
                                 this, SLOT(slotStop()),
                                 actionCollection(), "pageviewer_stop");

    //connect( this, SIGNAL(popupMenu(const QString &, const QPoint &)), this, SLOT(slotPopupMenu(const QString &, const QPoint &)));

    m_backAction->setEnabled(false);
    m_forwardAction->setEnabled(false);
    m_stopAction->setEnabled(false);
    
    connect(this, SIGNAL(started(KIO::Job *)), this, SLOT(slotStarted(KIO::Job* )));
    connect(this, SIGNAL(completed()), this, SLOT(slotCompleted()));
    connect(this, SIGNAL(canceled(const QString &)), this, SLOT(slotCancelled(const QString &)));

    connect(browserExtension(),
            SIGNAL(openURLRequest(const KURL&, const KParts::URLArgs&)),
            this,
            SLOT(formClicked(const KURL&, const KParts::URLArgs&)));
    
    m_current = m_history.end();
    m_restoring = false;
    // uncomment this to load konq plugins (doesn't work properly and clutters the GUI)
    //loadPlugins( partObject(), this, instance() );
    
}

PageViewer::~PageViewer()
{
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotBack()
{
    if ( m_current != m_history.begin() )
    {
        --m_current;
        m_restoring = true;
        openURL( (*m_current).url );
        m_restoring = false;
    }
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotForward()
{
    if (  m_current != m_history.fromLast() )
    {
        ++m_current;
        m_restoring = true;
        openURL( (*m_current).url );
        m_restoring = false;
    }
}

void PageViewer::slotBackAboutToShow()
{
    KPopupMenu *popup = m_backAction->popupMenu();
    popup->clear();

    if ( m_current == m_history.begin() )
        return;

    QValueList<PageViewerHistoryEntry>::Iterator it = m_current;
    --it;
    
    int i = 0;
    while( i < 10 )
    {
        if ( it == m_history.begin() )
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
    KPopupMenu *popup = m_forwardAction->popupMenu();
    popup->clear();

    if ( m_current == m_history.fromLast() )
        return;

    QValueList<PageViewerHistoryEntry>::Iterator it = m_current;
    ++it;
    
    int i = 0;
    while( i < 10 )
    {
        if ( it == m_history.fromLast() )
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

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
bool PageViewer::openURL(const KURL &url)
{
    Viewer::openURL(url);
    
    if (!m_restoring)
        addHistoryEntry(url);
    
    m_backAction->setEnabled( m_current != m_history.begin() );
    m_forwardAction->setEnabled( m_current != m_history.fromLast() );
  
    QString favicon = FeedIconManager::self()->iconLocation(url);
    if (!favicon.isEmpty()) 
        emit setTabIcon(QPixmap(KGlobal::dirs()->findResource("cache", favicon+".png")));
    else
        emit setTabIcon(SmallIcon("html"));
    
    return true;
}

void PageViewer::slotOpenURLRequest(const KURL& url, const KParts::URLArgs& args)
{
    if (args.frameName == "_blank") // apparently this indicates that the MMB was pressed...
        Viewer::slotOpenURLRequest(url, args);
    else
        openURL(url);
}

void PageViewer::slotPopupActivated( int id )
{
    QValueList<PageViewerHistoryEntry>::Iterator it = m_history.begin();
    while( it != m_history.end() )
    {
        if ( (*it).id == id )
        {
            m_current = it;
            m_restoring = true;
            openURL( (*m_current).url );
            m_restoring = false;
            return;
        }
        ++it;
    }
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::addHistoryEntry(const KURL& url)
{
    QValueList<PageViewerHistoryEntry>::Iterator it = m_current;
    
    // if We're not already the last entry, we truncate the list here before adding an entry
    if ( it != m_history.end() && it != m_history.fromLast() )
    {
        m_history.erase( ++it, m_history.end() );
    }
    PageViewerHistoryEntry newEntry( url, url.url() );
    kdDebug() << "PageViewer::addHistoryEntry() " << url.url() << endl;
    
    // Only save the new entry if it is different from the last
    if ( newEntry.url != (*m_current).url )
    {
        m_history.append( newEntry );
        m_current = m_history.fromLast();
    }
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotStarted( KIO::Job * )
{
    m_stopAction->setEnabled(true);
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotCompleted( )
{
    m_stopAction->setEnabled(false);
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotCancelled( const QString & /*errMsg*/ )
{
    m_stopAction->setEnabled(false);
}


void PageViewer::slotSetCaption(const QString& cap) {
    m_caption = cap;
    (*m_current).title = cap;
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
    grp.addBookmark(mgr, m_caption, toplevelURL());
    mgr->save();
}


void PageViewer::slotPopupMenu(KXMLGUIClient*, const QPoint& p, const KURL& kurl, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags, mode_t)
{
    m_url = kurl;
    QString url = kurl.url(); // maximal url confusion
    
    // if true show popup menu for link. Maybe that doesn't work properly when using frames
    bool isLink = kurl != Viewer::url();
    
//  KPopupMenu popup( i18n( "Documentation Viewer" ), this->widget() );
    KPopupMenu popup(this->widget());

//     bool needSep = false;
    int idNewWindow = -2;
    if (isLink)
    {
        idNewWindow = popup.insertItem(SmallIcon("tab_new"),i18n("Open Link in New &Tab"), this, SLOT(slotOpenLinkInForegroundTab()));
        popup.setWhatsThis(idNewWindow, i18n("<b>Open Link in New Tab</b><p>Opens current link in a new tab."));
        popup.insertItem(SmallIcon("window_new"), i18n("Open Link in External &Browser"), this, SLOT(slotOpenLinkInBrowser()));
                
        popup.insertSeparator();
        
        KAction *savelinkas = action("savelinkas");
        
        if (savelinkas)
                savelinkas->plug( &popup);
        
        KAction* copylinkaddress = action("copylinkaddress");
        if (copylinkaddress)
        {
            copylinkaddress->plug( &popup);
            popup.insertSeparator();
        }
    }
    else // we are not on a link
    {

        m_backAction->plug( &popup );
        m_forwardAction->plug( &popup );
        m_reloadAction->plug(&popup);
        m_stopAction->plug(&popup);

        popup.insertSeparator();
        action("viewer_copy")->plug(&popup);
        popup.insertSeparator();

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
        if (kurl.isValid())
            ;//             slotOpenInNewWindow(kurl);
//      openURL( kurl );
    }
}

void PageViewer::formClicked(const KURL& url, const KParts::URLArgs& args)
{
    if (args.doPost()) {
        browserExtension()->setURLArgs(args);
        openURL(url);
    }
}

#include "pageviewer.moc"
// vim: ts=4 sw=4 et
