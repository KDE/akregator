/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "akregator_part.h"
#include "akregator_view.h"
#include "akregatorconfig.h"
#include "fetchtransaction.h"

#include <kparts/browserinterface.h>
#include <kparts/genericfactory.h>
#include <kapplication.h>
#include <kinstance.h>
#include <kaction.h>
#include <kactionclasses.h>

#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kfiledialog.h>

#include <qfile.h>

using namespace Akregator;

typedef KParts::GenericFactory< aKregatorPart > aKregatorFactory;
K_EXPORT_COMPONENT_FACTORY( libakregatorpart, aKregatorFactory )

BrowserExtension::BrowserExtension( aKregatorPart *p, const char *name )
	    : KParts::BrowserExtension( p, name )
{
    m_part=p;
}

void BrowserExtension::saveSettings()
{
    m_part->saveSettings();
}

aKregatorPart::aKregatorPart( QWidget *parentWidget, const char * /*widgetName*/,
                              QObject *parent, const char *name, const QStringList& )
    : DCOPObject("aKregatorIface"), KParts::ReadWritePart(parent, name)
{
    // we need an instance
    setInstance( aKregatorFactory::instance() );

    m_totalUnread=0;
    m_loading=false;

    m_view=new aKregatorView(this, parentWidget, "Akregator View");
    m_extension=new BrowserExtension(this, "ak_extension");

    // notify the part that this is our internal widget
    setWidget(m_view);

    // create our actions
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    KStdAction::save(this, SLOT(save()), actionCollection());
    recentFilesAction = KStdAction::openRecent( this, SLOT(openURL(const KURL&)), actionCollection(), "file_open_recent" );

    new KAction(i18n("&Import Feeds..."), "", "", this, SLOT(fileImport()), actionCollection(), "file_import");

    /* -- ACTIONS */

    /* --- Feed popup menu */
    new KAction(i18n("&Add..."), "bookmark_add", "Insert", m_view, SLOT(slotFeedAdd()), actionCollection(), "feed_add");
    new KAction(i18n("Ne&w Folder..."), "folder_new", "Shift+Insert", m_view, SLOT(slotFeedAddGroup()), actionCollection(), "feed_add_group");
    new KAction(i18n("&Delete"), "editdelete", "Delete", m_view, SLOT(slotFeedRemove()), actionCollection(), "feed_remove");
    new KAction(i18n("&Edit"), "edit", "F2", m_view, SLOT(slotFeedModify()), actionCollection(), "feed_modify");
    new KAction(i18n("&Fetch"), "down", "Ctrl+F", m_view, SLOT(slotFetchCurrentFeed()), actionCollection(), "feed_fetch");
    new KAction(i18n("Fe&tch All"), "bottom", "Ctrl+L", m_view, SLOT(slotFetchAllFeeds()), actionCollection(), "feed_fetch_all");
    new KAction(i18n("Go to &Next Unread"), "", "N", m_view, SLOT(slotNextUnread()),actionCollection(), "feed_next_unread");
    new KAction(i18n("&Mark All as Read"), "", "Ctrl+R", m_view, SLOT(slotMarkAllRead()), actionCollection(), "feed_mark_all_as_read");
    new KAction(i18n("Ma&rk All Feeds as Read"), "", "Ctrl+Alt+R", m_view, SLOT(slotMarkAllFeedsRead()), actionCollection(), "feed_mark_all_feeds_as_read");
    new KAction(i18n("&Open Homepage"), "", "", m_view, SLOT(slotOpenHomepage()), actionCollection(), "feed_homepage");

    KRadioAction *ra=new KRadioAction(i18n("&Normal View"), "view_top_bottom", "Ctrl+1", m_view, SLOT(slotNormalView()), actionCollection(), "normal_view");
    ra->setExclusiveGroup( "ViewMode" );

    ra=new KRadioAction(i18n("&Widescreen View"), "view_left_right", "Ctrl+2", m_view, SLOT(slotWidescreenView()), actionCollection(), "widescreen_view");
    ra->setExclusiveGroup( "ViewMode" );

    ra=new KRadioAction(i18n("C&ombined View"), "view_text", "Ctrl+3", m_view, SLOT(slotCombinedView()), actionCollection(), "combined_view");
    ra->setExclusiveGroup( "ViewMode" );

    // set our XML-UI resource file
    setXMLFile("akregator_part.rc");

    readRecentFileEntries();

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);

    connect(parent, SIGNAL(markAllFeedsRead()), m_view, SLOT(slotMarkAllFeedsRead()));
}

void aKregatorPart::saveSettings()
{
   kdDebug() << "savesettings called"<<endl;
   m_view->saveSettings(true);
}

aKregatorPart::~aKregatorPart()
{
   saveSettings();
}

void aKregatorPart::readRecentFileEntries()
{
   KConfig *config = new KConfig("akregatorrc"); // move to shell!
   recentFilesAction->loadEntries(config,"Recent Files");
   delete config;
}

void aKregatorPart::setReadWrite(bool rw)
{
    ReadWritePart::setReadWrite(rw);
}

void aKregatorPart::setModified(bool modified)
{
    // get a handle on our Save action and make sure it is valid
    KAction *save = actionCollection()->action(KStdAction::stdName(KStdAction::Save));
    if (!save)
        return;

    // if so, we either enable or disable it based on the current
    // state
    if (modified)
        save->setEnabled(true);
    else
        save->setEnabled(false);

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
}

void aKregatorPart::changePart(KParts::ReadOnlyPart *p)
{
    emit partChanged(p);
}

void aKregatorPart::setCaption(const QString &text)
{
   emit setWindowCaption(text);
}

void aKregatorPart::setStatusBar(const QString &text)
{
   emit setStatusBarText(text);
}

void aKregatorPart::setProgress(int percent)
{
    emit m_extension->loadingProgress(percent);
}

void aKregatorPart::setStarted()
{
    emit started(0);
}

void aKregatorPart::setCompleted()
{
    emit completed(0);
}

void aKregatorPart::setCanceled(const QString &s)
{
    emit canceled(s);
}

void aKregatorPart::setTotalUnread(int unread)
{
    if (m_totalUnread != unread)
    {
        if (m_extension->browserInterface())
            m_extension->browserInterface()->callMethod( "updateUnread(int)", unread );
        m_totalUnread=unread;
    }
}

/*************************************************************************************************/
/* LOAD                                                                                          */
/*************************************************************************************************/

bool aKregatorPart::openURL(const KURL& url)
{
    recentFilesAction->addURL(url);
    
    // stop whatever we're doing before opening a new feed list
    if (m_loading)
    {
        m_view->endOperation();
        m_view->stopLoading();
        m_delayURL=url;
        QTimer::singleShot(1000, this, SLOT(openURLDelayed())); 
        return true;
    }
    else if (m_view->transaction()->isRunning())
    {
        m_view->endOperation();
        m_view->transaction()->stop();
        m_delayURL=url;
        QTimer::singleShot(1000, this, SLOT(openURLDelayed())); 
        return true;
    }

   return inherited::openURL(url);
}

void aKregatorPart::openURLDelayed()
{
    inherited::openURL(m_delayURL);
}

void aKregatorPart::openLastFeedList()
{
    openURL(Settings::lastOpenFile());
}

bool aKregatorPart::openFile()
{
    // m_file is always local so we can use QFile on it
    QFile file(m_file);
    if (file.open(IO_ReadOnly) == false)
    {
        return false;
    }

    m_loading=true;
    m_view->startOperation();
    setStatusBar( i18n("Opening Feed List...") );
    kapp->processEvents();

    // Read OPML feeds list and build QDom tree.
    QTextStream stream(&file);
    stream.setEncoding(QTextStream::UnicodeUTF8); // FIXME not all opmls are in utf8
    QDomDocument doc;
    QString str;

    str = stream.read();

    file.close();

    if (!doc.setContent(str))
    {
        m_view->operationError(i18n("Invalid Feed List"));
        return false;
    }

    if (!m_view->loadFeeds(doc)) // will take care of building feeds tree and loading archive
    {
        m_view->operationError(i18n("Invalid Feed List"));
        return false;
    }

    m_loading=false;
    m_view->endOperation();
    setStatusBar( QString::null );

    if (Settings::fetchOnStartup() && m_extension->browserInterface())
    {
        kdDebug() << "fetching on startup.." << endl;
        // has the shell loaded up a window already? then its not starting up
       QVariant shellHaveWindowLoaded = m_extension->browserInterface()->property( "haveWindowLoaded" );
       if (!shellHaveWindowLoaded.toBool())
            m_view->slotFetchAllFeeds();
    }
    return true;
}

bool aKregatorPart::closeURL()
{
    m_view->endOperation();
    if (m_loading)
    {
        m_view->stopLoading();
        return true;
    }
    else if (m_view->transaction()->isRunning())
    {
        m_view->transaction()->stop();
        return true;
    }
    
   return KParts::ReadWritePart::closeURL(); 
}

/*************************************************************************************************/
/* SAVE                                                                                          */
/*************************************************************************************************/

bool aKregatorPart::saveFile()
{
    // if we aren't read-write, return immediately
    if (isReadWrite() == false)
        return false;

    // m_file is always local, so we use QFile
    QFile file(m_file);
    if (file.open(IO_WriteOnly) == false)
        return fileSaveAs();

    // use QTextStream to dump the text to the file
    QTextStream stream(&file);
    stream.setEncoding(QTextStream::UnicodeUTF8);

    // Write OPML data file.
    // Archive data files are saved elsewhere.

    QDomDocument newdoc;
    QDomElement root = newdoc.createElement( "opml" );
    root.setAttribute( "version", "1.0" );
    newdoc.appendChild( root );

    QDomElement head = newdoc.createElement( "head" );
    root.appendChild( head );

    QDomElement title = newdoc.createElement( "text" );
    head.appendChild( title );

    QDomText t = newdoc.createTextNode( "aKregator Feeds" );
    title.appendChild( t );

    QDomElement body = newdoc.createElement( "body" );
    root.appendChild( body );

    m_view->storeTree( body, newdoc);

    stream << newdoc.toString();

    file.close();

    return true;
}

void aKregatorPart::importFile(QString file_name)
{
    QFile file(file_name);
    if (file.open(IO_ReadOnly) == false)
        return;

    // Read OPML feeds list and build QDom tree.
    QDomDocument doc;
    if (!doc.setContent(file.readAll())) {
        kdDebug() << "Failed to build DOM tree, is " << file_name << " valid XML?" << endl;
        return;
    }

    if (m_view->importFeeds(doc))
        setModified(true);
}


/*************************************************************************************************/
/* SLOTS                                                                                         */
/*************************************************************************************************/

void aKregatorPart::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString file_name = KFileDialog::getOpenFileName( QString::null,
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );

    if (file_name.isEmpty() == false)
        openURL(file_name);
}

bool aKregatorPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString file_name = KFileDialog::getSaveFileName( QString::null,
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );
    if (file_name.isEmpty() == false)
    {
        saveAs(file_name);
        return true;
    }
    return false;
}

void aKregatorPart::fileImport()
{
    QString file_name = KFileDialog::getOpenFileName( locate( "appdata", "kde.opml" ),
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );

    if (file_name.isEmpty() == false)
        importFile(file_name);
}

void aKregatorPart::fetchFeedUrl(const QString&s)
{
    kdDebug() << "fetchFeedURL=="<<s<<endl;
}

void aKregatorPart::addFeedToGroup(const QString& url, const QString& group)
{
    kdDebug() << "aKregatorPart::addFeedToGroup adding feed with URL " << url << " to group " << group << endl;
    m_view->addFeedToGroup(url, group);
}

/*************************************************************************************************/
/* STATIC METHODS                                                                                */
/*************************************************************************************************/

KAboutData* aKregatorPart::s_about = 0L;

KAboutData *aKregatorPart::createAboutData()
{
    if ( !s_about ) {
        s_about = new KAboutData("akregatorpart", I18N_NOOP("aKregatorPart"), "0.9",
                                 I18N_NOOP("This is a KPart for RSS aggregator"),
                                 KAboutData::License_GPL, "(C) 2004 Stanislav Karchebny", 0,
                                 "http://berk.upnet.ru/projects/kde/akregator",
                                 "Stanislav.Karchebny@kdemail.net");
        s_about->addAuthor("Stanislav Karchebny", I18N_NOOP("Author, Developer, Maintainer"),
                           "Stanislav.Karchebny@kdemail.net");
        s_about->addAuthor("Sashmit Bhaduri", I18N_NOOP("Developer"), "smt@vfemail.net");
    }
    return s_about;
}

#include "akregator_part.moc"

// vim: set et ts=4 sts=4 sw=4:
