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
#include <kmessagebox.h>

#include <qfile.h>
#include <private/qucomextra_p.h>

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
    recentFilesAction = KStdAction::openRecent( this, SLOT(openURL(const KURL&)), actionCollection(), "file_open_recent" );
 
    new KAction(i18n("&Import Feeds..."), "", "", this, SLOT(fileImport()), actionCollection(), "file_import");
    new KAction(i18n("&Export Feeds..."), "", "", this, SLOT(fileExport()), actionCollection(), "file_export");

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
//    setModified(false);

    connect(parent, SIGNAL(markAllFeedsRead()), m_view, SLOT(slotMarkAllFeedsRead()));
}

void aKregatorPart::saveSettings()
{
   kdDebug() << "savesettings called"<<endl;
   m_view->saveSettings(true);
}

aKregatorPart::~aKregatorPart()
{}

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
    // autosave
    if (url().isValid() && modified)
        saveFile();
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


// will do systray notification
void aKregatorPart::newArticle(Feed *src, const MyArticle &a)
{
    // HACK Because m_extension->browserInterface()->callMethod isn't flexible enough for us.

    if (m_extension->browserInterface())
    {
        int slot = m_extension->browserInterface()->metaObject()->findSlot( "newArticle(const QString&,const QPixmap&,const QString&)" );
        
        QUObject o[ 4 ];
        static_QUType_QString.set( o + 1, src->title() );
        static_QUType_ptr.set( o + 2, &(src->favicon()) );
        static_QUType_QString.set( o + 3, a.title() );
        m_extension->browserInterface()->qt_invoke( slot, o );
    }
}

void aKregatorPart::readProperties(KConfig* config)
{
    KURL u=config->readEntry("URL");
    if (u.isValid())
    {
        openURL(u);
        m_view->readProperties(config);
    }   
}

void aKregatorPart::saveProperties(KConfig* config)
{
    config->writeEntry("URL",url().url());
    m_view->saveProperties(config);
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

void aKregatorPart::openStandardFeedList()
{
    openURL(KGlobal::dirs()->saveLocation("data", "akregator/data") + "/feeds.opml");
}

bool aKregatorPart::isStandardFeedList()
{
    QString stdF="file:"+KGlobal::dirs()->saveLocation("data", "akregator/data") + "/feeds.opml";
    if (url().url()==stdF)
        return true;
    return false;
}

bool aKregatorPart::populateStandardFeeds()
{
    QString stdF=KGlobal::dirs()->saveLocation("data", "akregator/data") + "/feeds.opml";
    QFile file(stdF);

    if ( !file.open( IO_WriteOnly ) ) {
        return false;
    }
    QTextStream stream( &file );
    stream.setEncoding(QTextStream::UnicodeUTF8);
    
    QDomDocument doc;
    QDomProcessingInstruction z = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild( z );
    
    QDomElement root = doc.createElement( "opml" );
    root.setAttribute("version","1.0");
    doc.appendChild( root );

    QDomElement head = doc.createElement( "head" );
    root.appendChild(head);

    QDomElement text = doc.createElement( "text" );
    text.appendChild(doc.createTextNode(i18n("Feeds")));
    head.appendChild(text);

    QDomElement body = doc.createElement( "body" );
    root.appendChild(body);

    QDomElement mainFolder = doc.createElement( "outline" );
    mainFolder.setAttribute("text","KDE");
    body.appendChild(mainFolder);
    
    QDomElement ak = doc.createElement( "outline" );
    ak.setAttribute("text",i18n("Akregator News"));
    ak.setAttribute("xmlUrl","http://akregator.sf.net/rss2.php");
    mainFolder.appendChild(ak);

    QDomElement dot = doc.createElement( "outline" );
    dot.setAttribute("text",i18n("KDE Dot News"));
    dot.setAttribute("xmlUrl","http://www.kde.org/dotkdeorg.rdf");
    mainFolder.appendChild(dot);

    QDomElement plan = doc.createElement( "outline" );
    plan.setAttribute("text",i18n("Planet KDE"));
    plan.setAttribute("xmlUrl","http://planetkde.org/rss20.xml");
    mainFolder.appendChild(plan);

    QDomElement apps = doc.createElement( "outline" );
    apps.setAttribute("text",i18n("KDE Apps"));
    apps.setAttribute("xmlUrl","http://www.kde.org/dot/kde-apps-content.rdf");
    mainFolder.appendChild(apps);

    QDomElement look = doc.createElement( "outline" );
    look.setAttribute("text",i18n("KDE Look"));
    look.setAttribute("xmlUrl","http://www.kde.org/kde-look-content.rdf");
    mainFolder.appendChild(look);

    stream<<doc.toString();

    return true;
}

bool aKregatorPart::openFile()
{
    // m_file is always local so we can use QFile on it
    QFile file(m_file);
    if (file.open(IO_ReadOnly) == false)
    {
        if (isStandardFeedList())
        {
            if (populateStandardFeeds())
            {
                if (file.open(IO_ReadOnly) == false)
                    return false;
            }
            else    
                return false;
        }
        else
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

void aKregatorPart::importFile(QString fileName)
{
    QFile file(fileName);
    if (file.open(IO_ReadOnly) == false)
        return;

    // Read OPML feeds list and build QDom tree.
    QDomDocument doc;
    if (!doc.setContent(file.readAll())) {
        kdDebug() << "Failed to build DOM tree, is " << fileName << " valid XML?" << endl;
        return;
    }

    if (m_view->importFeeds(doc))
        setModified(true);
}

void aKregatorPart::exportFile(QString fileName)
{
   // we could KIO here instead of QFile
    QFile file(fileName);
    if ( file.exists() )
    
        if ( KMessageBox::questionYesNo(m_view,
          i18n("The file %1 already exists; do you want to overwrite it?").arg(fileName),
        i18n("Export"),   
        i18n("Overwrite"),
        i18n("Cancel")) == KMessageBox::No )
            return;
    if ( !file.open(IO_WriteOnly) )
    {
        KMessageBox::error(m_view, i18n("Access denied: cannot write to file %1").arg(fileName), i18n("Write error") ); 
        return;
    }
    
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
    QString file_name = KFileDialog::getOpenFileName( QString::null,
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );

    if (file_name.isEmpty() == false)
        importFile(file_name);
}

void aKregatorPart::fileExport()
{
    QString file_name = KFileDialog::getSaveFileName( QString::null,
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );
                        
    if ( !file_name.isEmpty() )
        exportFile(file_name);
}

void aKregatorPart::fetchAllFeeds()
{
    m_view->slotFetchAllFeeds();
}

void aKregatorPart::fetchFeedUrl(const QString&s)
{
    kdDebug() << "fetchFeedURL=="<<s<<endl;
}

void aKregatorPart::addFeedToGroup(const QString& url, const QString& group)
{
    kdDebug() << "aKregatorPart::addFeedToGroup adding feed with URL " << url << " to group " << group << endl;
    m_view->addFeedToGroup(url, group);
    setModified(true);
}

/*************************************************************************************************/
/* STATIC METHODS                                                                                */
/*************************************************************************************************/

KAboutData* aKregatorPart::s_about = 0L;

KAboutData *aKregatorPart::createAboutData()
{
    if ( !s_about ) {
	// this is semi-dummy about data, please see main.cpp for detailed one
        s_about = new KAboutData("akregatorpart", I18N_NOOP("aKregatorPart"), "1.0-beta6 \"Bezerkus\"",
                                 I18N_NOOP("This is a KPart for a feed aggregator"),
                                 KAboutData::License_GPL, "(C) 2004 akregator developers", 0,
                                 "http://akregator.sourceforge.net",
                                 "akregator-devel@lists.sf.net");
    }
    return s_about;
}

#include "akregator_part.moc"

// vim: set et ts=4 sts=4 sw=4:
