/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include <kaboutdata.h>
#include <kaction.h>
#include <kactionclasses.h>
#include <kactioncollection.h> 
#include <kapplication.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <khtmldefaults.h>
#include <kinstance.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <knotifydialog.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kparts/browserinterface.h>
#include <kparts/genericfactory.h>
#include <kparts/partmanager.h>

#include <qfile.h>
#include <qobjectlist.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qwidgetlist.h>
#include <private/qucomextra_p.h>

#include "aboutdata.h"
#include "akregator_part.h"
#include "akregator_view.h"
#include "akregatorconfig.h"
#include "akregator.h"
#include "fetchtransaction.h"
#include "frame.h"
#include "myarticle.h"
#include "notificationmanager.h"
#include "pageviewer.h"
#include "settings_appearance.h"
#include "settings_archive.h"
#include "settings_browser.h"
#include "settings_general.h"
#include "trayicon.h"

namespace Akregator {

typedef KParts::GenericFactory<Part> AkregatorFactory;
K_EXPORT_COMPONENT_FACTORY( libakregatorpart, AkregatorFactory )

BrowserExtension::BrowserExtension(Part *p, const char *name)
	    : KParts::BrowserExtension( p, name )
{
    m_part=p;
}

void BrowserExtension::saveSettings()
{
    m_part->saveSettings();
}

void Part::setupActions()
{
    // file menu

    new KAction(i18n("&Import Feeds..."), "", "", this, SLOT(fileImport()), actionCollection(), "file_import");
    new KAction(i18n("&Export Feeds..."), "", "", this, SLOT(fileExport()), actionCollection(), "file_export");
        
    /* --- Feed/Feed Group popup menu */
    new KAction(i18n("&Open Homepage"), "", "Ctrl+H", m_view, SLOT(slotOpenHomepage()), actionCollection(), "feed_homepage");
    new KAction(i18n("&Add Feed..."), "bookmark_add", "Insert", m_view, SLOT(slotFeedAdd()), actionCollection(), "feed_add");
    new KAction(i18n("Ne&w Folder..."), "folder_new", "Shift+Insert", m_view, SLOT(slotFeedAddGroup()), actionCollection(), "feed_add_group");
    new KAction(i18n("&Delete Feed"), "editdelete", "Alt+Delete", m_view, SLOT(slotFeedRemove()), actionCollection(), "feed_remove");
    new KAction(i18n("&Edit Feed..."), "edit", "F2", m_view, SLOT(slotFeedModify()), actionCollection(), "feed_modify");

    KActionMenu* vm = new KActionMenu( i18n( "&View Mode" ), actionCollection(), "view_mode" );
    
    KRadioAction *ra = new KRadioAction(i18n("&Normal View"), "view_top_bottom", "Ctrl+Shift+1", m_view, SLOT(slotNormalView()), actionCollection(), "normal_view");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);
    
    ra = new KRadioAction(i18n("&Widescreen View"), "view_left_right", "Ctrl+Shift+2", m_view, SLOT(slotWidescreenView()), actionCollection(), "widescreen_view");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);
    
    ra = new KRadioAction(i18n("C&ombined View"), "view_text", "Ctrl+Shift+3", m_view, SLOT(slotCombinedView()), actionCollection(), "combined_view");
    ra->setExclusiveGroup( "ViewMode" );
    vm->insert(ra);

    // toolbar / feed menu
    new KAction(i18n("&Fetch Feed"), "down", "Ctrl+L", m_view, SLOT(slotFetchCurrentFeed()), actionCollection(), "feed_fetch");
    new KAction(i18n("Fe&tch All Feeds"), "bottom", "Ctrl+Shift+L", m_view, SLOT(slotFetchAllFeeds()), actionCollection(), "feed_fetch_all");
    new KAction(i18n( "&Abort Fetches" ), "stop", Key_Escape, this, SLOT( slotStop() ), actionCollection(), "feed_stop");

    new KAction(i18n("&Mark Feed as Read"), "apply", "Ctrl+R", m_view, SLOT(slotMarkAllRead()), actionCollection(), "feed_mark_all_as_read");
    new KAction(i18n("Ma&rk All Feeds as Read"), "apply", "Ctrl+Shift+R", m_view, SLOT(slotMarkAllFeedsRead()), actionCollection(), "feed_mark_all_feeds_as_read");

    // "Go" menu

    new KAction(i18n("&Previous Feed"), "", "P", m_view, SLOT(slotPrevFeed()),actionCollection(), "go_prev_feed");
    new KAction(i18n("&Next Feed"), "", "N", m_view, SLOT(slotNextFeed()),actionCollection(), "go_next_feed");
    new KAction(i18n("N&ext Unread Feed"), "", "Alt+Plus", m_view, SLOT(slotNextUnreadFeed()),actionCollection(), "go_next_unread_feed");
    new KAction(i18n("Prev&ious Unread Feed"), "", "Alt+Minus", m_view, SLOT(slotPrevUnreadFeed()),actionCollection(), "go_prev_unread_feed");

    // Settings menu
    KToggleAction* sqf = new KToggleAction(i18n("Show Quick Filter"), QString::null, 0, m_view, SLOT(slotToggleShowQuickFilter()), actionCollection(), "show_quick_filter");
    sqf->setChecked( Settings::showQuickFilter() );
    KStdAction::configureNotifications(this,SLOT(showKNotifyOptions()), actionCollection()); // options_configure_notifications

    new KAction( i18n("Configure &Akregator..."), "configure", "", this, SLOT(showOptions()), actionCollection(), "akregator_configure_akregator" );
    //KStdAction::preferences( this, SLOT(showOptions()), actionCollection(), "akregator_configure_akregator" );

    // feed tree navigation
    new KAction( i18n("Go Up in Tree"), QString::null, "Alt+Up", m_view, SLOT(slotFeedsTreeUp()), actionCollection(), "feedstree_up" );
    new KAction( i18n("Go Down in Tree"), QString::null, "Alt+Down", m_view, SLOT(slotFeedsTreeDown()), actionCollection(), "feedstree_down" );
    new KAction( i18n("Go Left in Tree"), QString::null, "Alt+Left", m_view, SLOT(slotFeedsTreeLeft()), actionCollection(), "feedstree_left" );
    new KAction( i18n("Go Right in Tree"), QString::null, "Alt+Right", m_view, SLOT(slotFeedsTreeRight()), actionCollection(), "feedstree_right" );
    new KAction( i18n("Go to Top of Tree"), QString::null, "Alt+Home", m_view, SLOT(slotFeedsTreeHome()), actionCollection(), "feedstree_home" );
    new KAction( i18n("Go to Bottom of Tree"), QString::null, "Alt+End", m_view, SLOT(slotFeedsTreeEnd()), actionCollection(), "feedstree_end" );
    new KAction( i18n("Move Node Up"), QString::null, "Shift+Alt+Up", m_view, SLOT(slotMoveCurrentNodeUp()), actionCollection(), "feedstree_move_up" );
    new KAction( i18n("Move Node Down"), QString::null,  "Shift+Alt+Down", m_view, SLOT(slotMoveCurrentNodeDown()), actionCollection(), "feedstree_move_down" );
    new KAction( i18n("Move Node Left"), QString::null, "Shift+Alt+Left", m_view, SLOT(slotMoveCurrentNodeLeft()), actionCollection(), "feedstree_move_left" );
    new KAction( i18n("Move Node Right"), QString::null, "Shift+Alt+Right", m_view, SLOT(slotMoveCurrentNodeRight()), actionCollection(), "feedstree_move_right" );


    // article list
    new KAction( i18n("Open Article in Tab"), "tab_new", "Shift+Return", m_view, SLOT(slotOpenCurrentArticle()), actionCollection(), "article_open" );
    new KAction( i18n("Open Article in Background Tab"), QString::null, "tab_new", m_view, SLOT(slotOpenCurrentArticleBackgroundTab()), actionCollection(), "article_open_background_tab" );
    new KAction( i18n("Open Article in External Browser"), "window_new", "Ctrl+Shift+Return", m_view, SLOT(slotOpenCurrentArticleExternal()), actionCollection(), "article_open_external" );
    new KAction( i18n("&Previous Article"), QString::null, "Left", m_view, SLOT(slotPreviousArticle()), actionCollection(), "go_previous_article" );
    new KAction( i18n("&Next Article"), QString::null, "Right", m_view, SLOT(slotNextArticle()), actionCollection(), "go_next_article" );
    new KAction(i18n("Pre&vious Unread Article"), "", Key_Minus, m_view, SLOT(slotPrevUnreadArticle()),actionCollection(), "go_prev_unread_article");
    new KAction(i18n("Ne&xt Unread Article"), "", Key_Plus, m_view, SLOT(slotNextUnreadArticle()),actionCollection(), "go_next_unread_article");
    KToggleAction* tkf = new KToggleAction(i18n("&Keep Article"), "flag", "Ctrl+K", m_view, SLOT(slotArticleToggleKeepFlag()), actionCollection(), "article_toggle_keep");
    tkf->setChecked(false);

    new KAction(i18n("&Delete Article"), "editdelete", "Delete", m_view, SLOT(slotArticleDelete()), actionCollection(), "article_delete");
    

    KActionMenu* statusMenu = new KActionMenu ( i18n( "&Mark Article" ),
                                    actionCollection(), "article_set_status" );

    statusMenu->insert(new KAction(KGuiItem(i18n("Mark Article as &Unread"), "",
                       i18n("Mark selected article as unread")),
    "Ctrl+U", m_view, SLOT(slotSetSelectedArticleUnread()),
    actionCollection(), "article_set_status_unread"));

    statusMenu->insert(new KAction(KGuiItem(i18n("Mark Article as &New"), "",
                        i18n("Mark selected article as new")),
    "Ctrl+N", m_view, SLOT(slotSetSelectedArticleNew()),
    actionCollection(), "article_set_status_new" ));
}

Part::Part( QWidget *parentWidget, const char * /*widgetName*/,
                              QObject *parent, const char *name, const QStringList& )
    : DCOPObject("AkregatorIface"), MyBasePart(parent, name), m_shuttingDown(false), m_parentWidget(parentWidget)
{
    m_mergedPart = 0;
    m_backedUpList = false;
    // we need an instance
    setInstance( AkregatorFactory::instance() );

    m_standardFeedList = KGlobal::dirs()->saveLocation("data", "akregator/data") + "/feeds.opml";

    m_standardListLoaded = false;
    m_loading = false;

    m_view = new Akregator::View(this, parentWidget, "akregator_view");
    m_extension = new BrowserExtension(this, "ak_extension");

    // notify the part that this is our internal widget
    setWidget(m_view);

    setupActions();
    
    m_trayIcon = new TrayIcon( getMainWindow() );
    connect(m_trayIcon, SIGNAL(showPart()), this, SIGNAL(showPart()));

    if ( isTrayIconEnabled() )
    {
        m_trayIcon->show();
        NotificationManager::self()->setWidget(m_trayIcon);
    }
    else
        NotificationManager::self()->setWidget(getMainWindow());

    connect( m_trayIcon, SIGNAL(quitSelected()),
            kapp, SLOT(quit())) ;

    KPopupMenu* traypop = m_trayIcon->contextMenu();
    
    action("feed_fetch_all")->plug(traypop, 1);
    action("akregator_configure_akregator")->plug(traypop, 1);
    
    connect( m_view, SIGNAL(signalUnreadCountChanged(int)), m_trayIcon, SLOT(slotSetUnread(int)) );
    
    connect(kapp, SIGNAL(shutDown()), this, SLOT(slotOnShutdown()));

    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, SIGNAL(timeout()), this, SLOT(slotSaveFeedList()));
    m_autosaveTimer->start(5*60*1000); // 5 minutes
    
    // set our XML-UI resource file
    setXMLFile("akregator_part.rc", true);
    
    initFonts();
}

void Part::slotOnShutdown()
{
    m_shuttingDown = true;
    m_autosaveTimer->stop();
    saveSettings();
    slotSaveFeedList();
    m_view->slotOnShutdown();
}

void Part::slotSettingsChanged()
{
    NotificationManager::self()->setWidget(isTrayIconEnabled() ? m_trayIcon : getMainWindow());

    RSS::FileRetriever::setUseCache(Settings::useHTMLCache());
    
    QStringList fonts;
    fonts.append(Settings::standardFont());
    fonts.append(Settings::fixedFont());
    fonts.append(Settings::sansSerifFont());
    fonts.append(Settings::serifFont());
    fonts.append(Settings::standardFont());
    fonts.append(Settings::standardFont());
    fonts.append("0");
    Settings::setFonts(fonts);

    if (Settings::minimumFontSize() > Settings::mediumFontSize())
        Settings::setMediumFontSize(Settings::minimumFontSize());
    saveSettings();
    emit signalSettingsChanged();
}
void Part::saveSettings()
{
    m_view->saveSettings();
}

Part::~Part()
{
    if (!m_shuttingDown)
        slotOnShutdown();
}

void Part::setCaption(const QString &text)
{
    emit setWindowCaption(text);
}

void Part::setStatusBar(const QString &text)
{
    emit setStatusBarText(text);
}

void Part::setProgress(int percent)
{
    emit m_extension->loadingProgress(percent);
}

void Part::setStarted(KParts::Part* part)
{
    if (part == this)
        actionCollection()->action("feed_stop")->setEnabled(true);
    
    emit started(0);
}

void Part::setCompleted(KParts::Part* part)
{
    if (part == this)
        actionCollection()->action("feed_stop")->setEnabled(false);
    
    emit completed(0);
}

void Part::setCanceled(KParts::Part* part, const QString &s)
{
    if (part == this)
        actionCollection()->action("feed_stop")->setEnabled(false);
    
    emit canceled(s);
}

void Part::readProperties(KConfig* config)
{
    m_backedUpList = false;
    openStandardFeedList();
    
    if(m_view)
        m_view->readProperties(config);
}

void Part::saveProperties(KConfig* config)
{
    if(m_view)
        m_view->saveProperties(config);
}

/****************************************************************************/
/* LOAD                                                                     */
/****************************************************************************/

bool Part::openURL(const KURL& url)
{
    // stop whatever we're doing before opening a new feed list
    if (m_loading)
    {
        m_view->endOperation();
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
    else
    {
        m_file = url.path();
        bool ret = openFile();
        return ret;
    }
}

void Part::openURLDelayed()
{
    m_file = m_delayURL.path();
    openFile();
}

void Part::openStandardFeedList()
{
    if ( !m_standardFeedList.isEmpty() && openURL(m_standardFeedList) )
        m_standardListLoaded = true;
}

QDomDocument Part::createDefaultFeedList()
{
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

    return doc;
}

bool Part::openFile()
{
    QString str;
    // m_file is always local so we can use QFile on it
    QFile file(m_file);
    if (!file.exists())
    {
        m_view->loadFeeds(createDefaultFeedList());
    }
    else
    {
        if (file.open(IO_ReadOnly))
        {
            // Read OPML feeds list and build QDom tree.
            QTextStream stream(&file);
            stream.setEncoding(QTextStream::UnicodeUTF8); // FIXME not all opmls are in utf8
            str = stream.read();
            file.close();
        }
        else
        {
            KMessageBox::error(m_view, i18n("Could not read standard feed list (%1). A default feed list will be used.").arg(m_file), i18n("Read Error") );
            return false;
        }

        setStatusBar( i18n("Opening Feed List...") );

        QDomDocument doc;

        if (!doc.setContent(str))
        {
            QString backup = m_file + "-backup." +  QString::number(QDateTime::currentDateTime().toTime_t());

            copyFile(backup);

            KMessageBox::error(m_view, i18n("<qt>The standard feed list is corrupted (invalid XML). A backup was created:<p><b>%2</b></p></qt>").arg(backup), i18n("XML Parsing Error") );

            doc = createDefaultFeedList();
        }

        if (!m_view->loadFeeds(doc))
        {
            QString backup = m_file + "-backup." +  QString::number(QDateTime::currentDateTime().toTime_t());
            copyFile(backup);

            KMessageBox::error(m_view, i18n("<qt>The standard feed list is corrupted (no valid OPML). A backup was created:<p><b>%2</b></p></qt>").arg(backup), i18n("OPML Parsing Error") );
            m_view->loadFeeds(createDefaultFeedList());
        }
        
        setStatusBar(QString::null);
    }
    
    if( Settings::markAllFeedsReadOnStartup() )
        m_view->slotMarkAllFeedsRead();

    if (Settings::fetchOnStartup() && m_extension->browserInterface())
    {
        // has the shell loaded up a window already? then its not starting up
       QVariant shellHaveWindowLoaded = m_extension->browserInterface()->property( "haveWindowLoaded" );
       if (!shellHaveWindowLoaded.toBool())
            m_view->slotFetchAllFeeds();
    }
    return true;
}

bool Part::closeURL()
{
    m_view->endOperation();
    setStatusBar(QString::null);

    if (m_loading)
    {
        m_loading = false;
        kdDebug() << "closeURL: stop loading" << endl;
        return true;
    }
    else if (m_view->transaction()->isRunning())
    {
        m_view->transaction()->stop();
        kdDebug() << "closeURL: stop transaction" << endl;
        return true;
    }

   return MyBasePart::closeURL();
}


void Part::slotSaveFeedList()
{
    // don't save to the standard feed list, when it wasn't completely loaded before
    if (!m_standardListLoaded)
        return;
    
    // the first time we overwrite the feed list, we create a backup
    if (!m_backedUpList)
    {
        QString backup = m_file + "~";

        if (copyFile(backup))
            m_backedUpList = true;
    }

    QFile file(m_file);    
    if (file.open(IO_WriteOnly) == false)
    {
        //FIXME: allow to save the feedlist into different location -tpr 20041118
        KMessageBox::error(m_view, i18n("Access denied: cannot save feed list (%1)").arg(m_file), i18n("Write error") );
        return;
    }

    // use QTextStream to dump the text to the file
    QTextStream stream(&file);
    stream.setEncoding(QTextStream::UnicodeUTF8);

    // Write OPML data file.
    // Archive data files are saved elsewhere.

    stream << m_view->feedListToOPML().toString();

    file.close();
}

bool Part::isTrayIconEnabled() const
{
    return Settings::showTrayIcon();
}

QPixmap Part::takeTrayIconScreenshot() const
{
    return m_trayIcon->takeScreenshot();
}

bool Part::mergePart(KParts::Part* part)
{
    if (part != m_mergedPart)
    {
        if (!factory())
        {
            kdDebug() << "Akregator::Part::mergePart(): factory() returns NULL" << endl;
            return false;
        }
        if (m_mergedPart)
            factory()->removeClient(m_mergedPart);
        if (part)
            factory()->addClient(part);
    
        m_mergedPart = part;
    }
    return true;
}

QWidget* Part::getMainWindow()
{
        // this is a dirty fix to get the main window used for the tray icon

        QWidgetList *l = kapp->topLevelWidgets();
        QWidgetListIt it( *l );
        QWidget *wid;

        // check if there is an akregator main window
        while ( (wid = it.current()) != 0 )
        {
        ++it;
        //kdDebug() << "win name: " << wid->name() << endl;
        if (QString(wid->name()) == "akregator_mainwindow")
            return wid;
        }
        // if not, check for kontact main window
        QWidgetListIt it2( *l );
        while ( (wid = it2.current()) != 0 )
        {
            ++it2;
            if (QString(wid->name()).startsWith("kontact-mainwindow"))
                return wid;
        }
    return 0;
}


void Part::importFile(const QString& fileName)
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

    m_view->importFeeds(doc);
}

void Part::exportFile(const QString& fileName)
{
   // TODO we could KIO here instead of QFile
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

    QDomDocument doc = m_view->feedListToOPML();
    
    stream << doc.toString();
    file.close();
}

/****************************************************************************/
/* SLOTS                                                                    */
/****************************************************************************/

void Part::fileOpen()
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
/*
bool Akregator::Part::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString file_name = KFileDialog::getSaveFileName( QString::null,
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );
    if (file_name.isEmpty() == false)
    {
        kdDebug() << "SaveAs called! Shouldn't ever happen!" << endl;
        //saveAs(file_name);
        return true;
    }
    return false;
}
*/

void Part::fileImport()
{
    QString file_name = KFileDialog::getOpenFileName( QString::null,
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );

    if (file_name.isEmpty() == false)
        importFile(file_name);
}

void Part::fileExport()
{
    QString file_name = KFileDialog::getSaveFileName( QString::null,
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );

    if ( !file_name.isEmpty() )
        exportFile(file_name);
}

void Part::fetchAllFeeds()
{
    m_view->slotFetchAllFeeds();
}

void Part::fetchFeedUrl(const QString&s)
{
    kdDebug() << "fetchFeedURL==" << s << endl;
}

void Part::addFeedsToGroup(const QStringList& urls, const QString& group)
{
    for (QStringList::ConstIterator it = urls.begin(); it != urls.end(); ++it)
    {
        kdDebug() << "Akregator::Part::addFeedToGroup adding feed with URL " << *it << " to group " << group << endl;
        m_view->addFeedToGroup(*it, group);
    }
    NotificationManager::self()->slotNotifyFeeds(urls);
}


/****************************************************************************/
/* STATIC METHODS                                                           */
/****************************************************************************/

KAboutData *Part::createAboutData()
{
    return new Akregator::AboutData;
}

void Part::showKNotifyOptions()
{
    KAboutData* about = new Akregator::AboutData;
    KNotifyDialog::configure(m_view, "akregator_knotify_config", about);
    delete about;
}

void Part::showOptions()
{
    if ( KConfigDialog::showDialog( "settings" ) )
        return;

    KConfigDialog *dialog = new KConfigDialog( m_view, "settings", Settings::self() );
    dialog->addPage(new SettingsGeneral(0, "General"), i18n("General"), "package_settings");
    dialog->addPage(new SettingsArchive(0, "Archive"), i18n("Archive"), "package_settings");
    dialog->addPage(new SettingsAppearance(0, "Appearance"), i18n("Appearance"), "fonts");
    dialog->addPage(new SettingsBrowser(0, "Browser"), i18n("Browser"), "package_network");
    connect( dialog, SIGNAL(settingsChanged()),
             this, SLOT(slotSettingsChanged()) );
    connect( dialog, SIGNAL(settingsChanged()),
             m_trayIcon, SLOT(settingsChanged()) );

    dialog->show();
}

void Part::partActivateEvent(KParts::PartActivateEvent* event)
{
    if (factory() && m_mergedPart)
    {
        if (event->activated())
            factory()->addClient(m_mergedPart);
        else
            factory()->removeClient(m_mergedPart);
    }

    MyBasePart::partActivateEvent(event);
}

KParts::Part* Part::hitTest(QWidget *widget, const QPoint &globalPos)
{
    bool child = false;
    QWidget *me = this->widget();
    while (widget) {
        if (widget == me) {
            child = true;
            break;
        }
        if (!widget) {
            break;
        }
        widget = widget->parentWidget();
    }
    if (m_view && m_view->currentFrame() && child) {
        return m_view->currentFrame()->part();
    } else {
        return MyBasePart::hitTest(widget, globalPos);
    }
}

void Part::initFonts()
{
    QStringList fonts = Settings::fonts();
    if (fonts.isEmpty())
    {
        fonts.append(KGlobalSettings::generalFont().family());
        fonts.append(KGlobalSettings::fixedFont().family());
        fonts.append(KGlobalSettings::generalFont().family());
        fonts.append(KGlobalSettings::generalFont().family());
        fonts.append("0");
    }
    Settings::setFonts(fonts);
    if (Settings::standardFont().isEmpty())
        Settings::setStandardFont(fonts[0]);
    if (Settings::fixedFont().isEmpty())
        Settings::setFixedFont(fonts[1]);
    if (Settings::sansSerifFont().isEmpty())
        Settings::setSansSerifFont(fonts[2]);
    if (Settings::serifFont().isEmpty())
        Settings::setSerifFont(fonts[3]);

    KConfig* conf = Settings::self()->config();
    conf->setGroup("HTML Settings");

    KConfig konq("konquerorrc", true, false);
    konq.setGroup("HTML Settings");
            
    if (!conf->hasKey("MinimumFontSize"))
    {
        int minfs;
        if (konq.hasKey("MinimumFontSize"))
            minfs = konq.readNumEntry("MinimumFontSize");
        else
            minfs = KGlobalSettings::generalFont().pointSize();
        kdDebug() << "Part::initFonts(): set MinimumFontSize to " << minfs << endl;
        Settings::setMinimumFontSize(minfs);
    }

    if (!conf->hasKey("MediumFontSize"))
    {
        int medfs;
        if (konq.hasKey("MediumFontSize"))
            medfs = konq.readNumEntry("MediumFontSize");
        else
            medfs = KGlobalSettings::generalFont().pointSize();
        kdDebug() << "Part::initFonts(): set MediumFontSize to " << medfs << endl;
        Settings::setMediumFontSize(medfs);
    }
    
    if (!conf->hasKey("UnderlineLinks"))
    {
        bool underline = true;
        if (konq.hasKey("UnderlineLinks"))
            underline = konq.readBoolEntry("UnderlineLinks");

        kdDebug() << "Part::initFonts(): set UnderlineLinks to " << underline << endl;
        Settings::setUnderlineLinks(underline);
    }
    
}

bool Part::copyFile(const QString& backup)
{
    QFile file(m_file);

    if (file.open(IO_ReadOnly))
    {
        QFile backupFile(backup);
        if (backupFile.open(IO_WriteOnly))
        {
            QTextStream in(&file);
            QTextStream out(&backupFile);
            while (!in.atEnd())
                out << in.readLine();
            backupFile.close();
            file.close();
            return true;
        }
        else
        {
            file.close();
            return false;
        }
    }
    return false;
}

} // namespace Akregator
#include "akregator_part.moc"

// vim: set et ts=4 sts=4 sw=4:
