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
#include "browserrun.h"
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
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kparts/browserinterface.h>

#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QList>
#include <QMetaObject>
#include <QString>
#include <QPixmap>


#include <cstdlib>
using std::abs;

namespace Akregator {


// taken from KDevelop
class PageViewer::HistoryEntry
{
    public:

    KUrl url;
    QString title;
    QByteArray state;
    int id;

    HistoryEntry() {}
    HistoryEntry(const KUrl& u, const QString& t=QString()): url(u), title(t)
    {
        id = abs( QTime::currentTime().msecsTo( QTime() ) );    // nasty, but should provide a reasonably unique number
    }

};

struct PageViewer::PageViewerPrivate
{
    QList<HistoryEntry> history;
    QList<HistoryEntry>::Iterator current;

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

    setXMLFile(KStandardDirs::locate("data", "akregator/pageviewer.rc"), true);

    d->backAction = new KToolBarPopupAction(i18n("Back"), "go-previous", "Alt+Left", this, SLOT(slotBack()), actionCollection(), "pageviewer_back");

    connect(d->backAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotBackAboutToShow()));
    connect(d->backAction->popupMenu(), SIGNAL(activated(int)),
            this, SLOT(slotPopupActivated(int)));


    d->forwardAction = new KToolBarPopupAction(i18n("Forward"), "go-next", "Alt+Right", this, SLOT(slotForward()), actionCollection(), "pageviewer_forward");

    connect(d->forwardAction->popupMenu(), SIGNAL(aboutToShow()),
            this, SLOT(slotForwardAboutToShow()));
    connect(d->forwardAction->popupMenu(), SIGNAL(activated(int)),
            this, SLOT(slotPopupActivated(int)));

    d->reloadAction = new KAction(i18n("Reload"), "view-refresh", 0,
                            this, SLOT(slotReload()),
                            actionCollection(), "pageviewer_reload");
    d->stopAction = new KAction(i18n("Stop"), "process-stop", 0,
                                 this, SLOT(slotStop()),
                                 actionCollection(), "pageviewer_stop");

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
        QList<HistoryEntry>::Iterator tmp = d->current;
        --tmp;
        restoreHistoryEntry(tmp);
    }
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::slotForward()
{
    if ( !d->history.isEmpty() && d->current != --(d->history.end()) )
    {
        QList<HistoryEntry>::Iterator tmp = d->current;
        ++tmp;
        restoreHistoryEntry(tmp);
    }
}

void PageViewer::slotBackAboutToShow()
{
    KMenu *popup = d->backAction->popupMenu();
    popup->clear();

    if ( d->current == d->history.begin() )
        return;

    QList<HistoryEntry>::Iterator it = d->current;
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
    KMenu *popup = d->forwardAction->popupMenu();
    popup->clear();

    if ( d->history.isEmpty() || d->current == --(d->history.end()) )
        return;

    QList<HistoryEntry>::Iterator it = d->current;
    ++it;

    int i = 0;
    while( i < 10 )
    {

        if ( !d->history.isEmpty() && it != --(d->history.end()) )
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
    openUrl( url() );
}

void PageViewer::slotStop()
{
    closeUrl();
}

void PageViewer::openPage(const KUrl& url)
{
    Viewer::openPage(url);

    addHistoryEntry(url);

    d->backAction->setEnabled( !d->history.isEmpty() && d->current != d->history.begin() );
    d->forwardAction->setEnabled( d->current != d->history.end() && d->current != --(d->history.end()) );

    QString favicon = FeedIconManager::self()->iconLocation(url);
    if (!favicon.isEmpty())
        emit setTabIcon(QPixmap(KGlobal::dirs()->findResource("cache", favicon+".png")));
    else
        emit setTabIcon(SmallIcon("html"));
}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
bool PageViewer::openUrl(const KUrl &url)
{
    updateHistoryEntry();
    new Akregator::BrowserRun(this, (QWidget*)parent(), this, url, browserExtension()->urlArgs());
    emit started(0);

    return true;
}

void PageViewer::slotOpenUrlRequest(const KUrl& url, const KParts::URLArgs& args)
{
    updateHistoryEntry();
    if (args.doPost())
    {
        browserExtension()->setURLArgs(args);
        openUrl(url);
    }

}

void PageViewer::urlSelected(const QString &url, int button, int state, const QString &_target, KParts::URLArgs args)
{
    updateHistoryEntry();
    if (button == Qt::MidButton)
        Viewer::urlSelected(url, button, state, _target, args);
    else
    {
        browserExtension()->setURLArgs(args);
        if (_target.toLower() == "_blank")
            Viewer::urlSelected(url, button, state, _target, args);
        else
            openUrl(completeURL(url) );
    }
}

void PageViewer::slotPopupActivated( int id )
{
    QList<HistoryEntry>::Iterator it = d->history.begin();
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
    if (d->current != d->history.end())
    {
        (*d->current).title = d->caption;
        (*d->current).state.clear(); // Start with empty buffer.
        QDataStream stream( &((*d->current).state),QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_3_1);
        browserExtension()->saveState(stream);
    }
}

void PageViewer::restoreHistoryEntry(const QList<HistoryEntry>::Iterator& entry)
{
    updateHistoryEntry();
    QDataStream stream( &((*entry).state),QIODevice::ReadOnly );
    stream.setVersion(QDataStream::Qt_3_1);
    browserExtension()->restoreState( stream );
    d->current = entry;
    d->backAction->setEnabled( !d->history.isEmpty() && d->current != d->history.begin() );
    d->forwardAction->setEnabled(  d->current != d->history.end() && d->current != --(d->history.end()) );
    //openUrl( entry.url ); // TODO read state


}

// Taken from KDevelop (lib/widgets/kdevhtmlpart.cpp)
void PageViewer::addHistoryEntry(const KUrl& url)
{
    QList<HistoryEntry>::Iterator it = d->current;

    // if We're not already the last entry, we truncate the list here before adding an entry
    if ( it != d->history.end()  && it != --(d->history.end()) )
    {
        d->history.erase( ++it, d->history.end() );
    }
    HistoryEntry newEntry( url, url.url() );

    // Only save the new entry if it is different from the last
    if ( it != d->history.end() && newEntry.url != (*d->current).url )
    {
        d->history.append( newEntry );
        d->current = --(d->history.end());
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


void PageViewer::slotSetCaption(const QString& cap)
{
    d->caption = cap;
    if (d->current != d->history.end())
        (*d->current).title = cap;
}

void PageViewer::slotPaletteOrFontChanged()
{
    kDebug() <<"PageViewer::slotPaletteOrFontChanged()";
    // taken from KonqView (kdebase/konqueror/konq_view.cc)
    QObject *obj = KParts::BrowserExtension::childObject(this);
    if ( !obj ) // not all views have a browser extension !
        return;

    QMetaObject::invokeMethod( obj, "reparseConfiguration()" );

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


void PageViewer::slotPopupMenu(const QPoint& p, const KUrl& kurl, mode_t, const KParts::URLArgs&, KParts::BrowserExtension::PopupFlags kpf)
{
    m_url = kurl;
    QString url = kurl.url(); // maximal url confusion

    const bool isLink = (kpf & KParts::BrowserExtension::ShowNavigationItems) == 0; // ## Why not use kpf & IsLink?

    KMenu popup(this->widget());

    int idNewWindow = -2;
    if (isLink)
    {
        idNewWindow = popup.insertItem(SmallIcon("tab-new"),i18n("Open Link in New &Tab"), this, SLOT(slotOpenLinkInForegroundTab()));
        popup.setWhatsThis(idNewWindow, i18n("<b>Open Link in New Tab</b><p>Opens current link in a new tab.</p>"));
        popup.insertItem(SmallIcon("window-new"), i18n("Open Link in External &Browser"), this, SLOT(slotOpenLinkInBrowser()));

        popup.addSeparator();
        action("savelinkas")->plug(&popup);
        KAction* copylinkaddress = action("copylinkaddress");
        if (copylinkaddress)
        {
            copylinkaddress->plug( &popup);
            //popup.addSeparator();
        }
    }
    else // we are not on a link
    {

        d->backAction->plug( &popup );
        d->forwardAction->plug( &popup );
        d->reloadAction->plug(&popup);
        d->stopAction->plug(&popup);

        popup.addSeparator();
        action("viewer_copy")->plug(&popup);
        popup.addSeparator();

        KAction* incFontAction = this->action("incFontSizes");
        KAction* decFontAction = this->action("decFontSizes");
        if ( incFontAction && decFontAction )
        {
            incFontAction->plug( &popup );
            decFontAction->plug( &popup );
            popup.addSeparator();
        }

        popup.insertItem(SmallIcon("window-new"), i18n("Open Page in External Browser"), this, SLOT(slotOpenLinkInBrowser()));

        action("viewer_print")->plug(&popup);
        popup.addSeparator();

        KAction *ac = action("setEncoding");
        if (ac)
            ac->plug(&popup);
        popup.insertItem(SmallIcon("bookmark-new"),i18n("Add to Konqueror Bookmarks"), this, SLOT(slotGlobalBookmarkArticle()));
    }

    popup.exec(p);
#ifdef __GNUC__
#warning port!
#endif
/*
    int r = popup.exec(p);

    if (r == idNewWindow)
    {
        KUrl kurl;
        if (!KUrl(url).path().startsWith("/"))
        {
            kDebug() <<"processing relative url:" << url;
            if (url.startsWith("#"))
            {
                kurl = KUrl(PageViewer::url());
                kurl.setRef(url.mid(1));
            }
            else
                kurl = KUrl(PageViewer::url().upUrl().url(true)+url);
        }
        else
            kurl = KUrl(url);
//    kurl.addPath(url);
        if (kurl.isValid())
            ;//             slotOpenInNewWindow(kurl);
//      openUrl( kurl );
    }
*/
}

} // namespace Akregator

#include "pageviewer.moc"
