/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#include "aboutdata.h"
#include "actionmanagerimpl.h"
#include "akregatorconfig.h"
#include "akregator_part.h"
#include "article.h"
#include "fetchqueue.h"
#include "feediconmanager.h"
#include "framemanager.h"
#include "kernel.h"
#include "mainwidget.h"
#include "notificationmanager.h"
#include "plugin.h"
#include "pluginmanager.h"
#include "storage.h"
#include "storagefactory.h"
#include "storagefactoryregistry.h"
#include "trayicon.h"
#include "dummystorage/storagefactorydummyimpl.h"

#include <knotifyconfigwidget.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <KSaveFile>
#include <kservice.h>
#include <kxmlguifactory.h>
#include <kio/netaccess.h>
#include <KParts/GenericFactory>
#include <KParts/Plugin>
#include <KCMultiDialog>
#include <kstandardaction.h>

#include <QFile>
#include <QObject>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#include <QDomDocument>
#include "partadaptor.h"

namespace Akregator {

static const KAboutData &createAboutData()
{
  static Akregator::AboutData about;
  return about;
}

K_PLUGIN_FACTORY(AkregatorFactory, registerPlugin<Part>();)
K_EXPORT_PLUGIN(AkregatorFactory(createAboutData()))

BrowserExtension::BrowserExtension(Part *p, const char *name)
	    : KParts::BrowserExtension( p)
{
    setObjectName(name);
    m_part=p;
}

void BrowserExtension::saveSettings()
{
    m_part->saveSettings();
}

Part::Part( QWidget *parentWidget, QObject *parent, const QVariantList& )
    : inherited(parent)
    , m_standardListLoaded(false)
    , m_shuttingDown(false)
    , m_backedUpList(false)
    , m_mainWidget(0)
    , m_storage(0)
    , m_dialog(0)

{
    setPluginLoadingMode( LoadPluginsIfEnabled );
    setPluginInterfaceVersion( AKREGATOR_PLUGIN_INTERFACE_VERSION );

    setComponentData( AkregatorFactory::componentData() );
    setXMLFile("akregator_part.rc", true);

    new PartAdaptor( this );
    QDBusConnection::sessionBus().registerObject("/Akregator", this);

    FeedIconManager::self(); // FIXME: registering the icon manager dbus iface here,
                               // because otherwise we get a deadlock later

    m_standardFeedList = KGlobal::dirs()->saveLocation("data", "akregator/data") + "/feeds.opml";

    Backend::StorageFactoryDummyImpl* dummyFactory = new Backend::StorageFactoryDummyImpl();
    Backend::StorageFactoryRegistry::self()->registerFactory(dummyFactory, dummyFactory->key());
    loadStoragePlugins(); // FIXME: also unload them!

    m_storage = 0;
    Backend::StorageFactory* storageFactory = Backend::StorageFactoryRegistry::self()->getFactory(Settings::archiveBackend());
    if (storageFactory != 0)
        m_storage = storageFactory->createStorage(QStringList());

    if (!m_storage) // Houston, we have a problem
    {
        m_storage = Backend::StorageFactoryRegistry::self()->getFactory("dummy")->createStorage(QStringList());

        KMessageBox::error(parentWidget, i18n("Unable to load storage backend plugin \"%1\". No feeds are archived.", Settings::archiveBackend()), i18n("Plugin error") );
    }

    m_storage->open(true);
    Kernel::self()->setStorage(m_storage);

    m_actionManager = new ActionManagerImpl(this);
    ActionManager::setInstance(m_actionManager);

    m_mainWidget = new Akregator::MainWidget(this, parentWidget, m_actionManager, "akregator_view");
    m_extension = new BrowserExtension(this, "ak_extension");

    connect(Kernel::self()->frameManager(), SIGNAL(signalCaptionChanged(const QString&)), this, SIGNAL(setWindowCaption(const QString&)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalStatusText(const QString&)), this, SIGNAL(setStatusBarText(const QString&)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalLoadingProgress(int)), m_extension, SIGNAL(loadingProgress(int)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalCanceled(const QString&)), this, SIGNAL(canceled(const QString&)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalStarted()), this, SLOT(slotStarted()));
    connect(Kernel::self()->frameManager(), SIGNAL(signalCompleted()), this, SIGNAL(completed()));

    // notify the part that this is our internal widget
    setWidget(m_mainWidget);

    TrayIcon* trayIcon = new TrayIcon( getMainWindow() );
    TrayIcon::setInstance(trayIcon);
    m_actionManager->initTrayIcon(trayIcon);

    connect(trayIcon, SIGNAL(toggleShowPart()), this, SIGNAL(toggleShowPart()));

    if ( isTrayIconEnabled() )
    {
        trayIcon->show();
#ifdef __GNUC__
#warning this needs a way to get the position of the systray
#endif
        NotificationManager::self()->setWidget(0, componentData());
    }
    else
        NotificationManager::self()->setWidget(getMainWindow(), componentData());

    connect( trayIcon, SIGNAL(quitSelected()),
            kapp, SLOT(quit())) ;

    connect( m_mainWidget, SIGNAL(signalUnreadCountChanged(int)), trayIcon, SLOT(slotSetUnread(int)) );

    connect(kapp, SIGNAL(aboutToQuit()), this, SLOT(slotOnShutdown()));

    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, SIGNAL(timeout()), this, SLOT(slotSaveFeedList()));
    m_autosaveTimer->start(5*60*1000); // 5 minutes

    initFonts();

    QString useragent = QString( "Akregator/%1; syndication" ).arg( AKREGATOR_VERSION );

    if( !Settings::customUserAgent().isEmpty() )
        useragent = Settings::customUserAgent();

    Syndication::FileRetriever::setUserAgent( useragent );
}


void Part::loadStoragePlugins()
{
    KService::List offers = PluginManager::query( "[X-KDE-akregator-plugintype] == 'storage'" );
    for ( KService::List::ConstIterator it = offers.begin(), end = offers.end(); it != end; ++it )
    {
        Akregator::Plugin* plugin = PluginManager::createFromService(*it);
        if (plugin)
            plugin->initialize();
    }
}

void Part::slotStarted()
{
    emit started(0L);
}

void Part::slotOnShutdown()
{
    m_shuttingDown = true;
    m_autosaveTimer->stop();
    saveSettings();
    slotSaveFeedList();
    m_mainWidget->slotOnShutdown();
    //delete m_mainWidget;
    delete TrayIcon::getInstance();
    TrayIcon::setInstance(0L);
    delete m_storage;
    m_storage = 0;
    //delete m_actionManager;
}

void Part::slotSettingsChanged()
{
#ifdef __GNUC__
#warning Tray icons are no longer widgets
#endif
//    NotificationManager::self()->setWidget(isTrayIconEnabled() ? TrayIcon::getInstance() : getMainWindow(), instance());
    NotificationManager::self()->setWidget( getMainWindow(), componentData());

    Syndication::FileRetriever::setUseCache(Settings::useHTMLCache());

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
    m_mainWidget->saveSettings();
}

Part::~Part()
{
    kDebug() <<"Part::~Part() enter";
    if (!m_shuttingDown)
        slotOnShutdown();
    delete m_dialog;
    kDebug() <<"Part::~Part(): leaving";
}

void Part::readProperties(const KConfigGroup & config)
{
    m_backedUpList = false;
    openStandardFeedList();

    if(m_mainWidget)
        m_mainWidget->readProperties(config);
}

void Part::saveProperties(KConfigGroup & config)
{
    if (m_mainWidget)
    {
        slotSaveFeedList();
        m_mainWidget->saveProperties(config);
    }
}

bool Part::openUrl(const KUrl& url)
{
    setLocalFilePath(url.path());
    return openFile();
}

void Part::openStandardFeedList()
{
    if ( !m_standardFeedList.isEmpty() && openUrl(KUrl::fromPath(m_standardFeedList)) )
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

    QDomElement akb = doc.createElement( "outline" );
    akb.setAttribute("text",i18n("Akregator Blog"));
    akb.setAttribute("xmlUrl","http://akregator.pwsp.net/blog/?feed=rss2");
    mainFolder.appendChild(akb);

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
    emit setStatusBarText(i18n("Opening Feed List...") );

    QString str;
    // m_file is always local so we can use QFile on it
    QFile file(localFilePath());

    bool fileExists = file.exists();
    QString listBackup = m_storage->restoreFeedList();

    QDomDocument doc;

    if (!fileExists)
    {
        doc = createDefaultFeedList();
    }
    else
    {
        if (file.open(QIODevice::ReadOnly))
        {
            // Read OPML feeds list and build QDom tree.
            QTextStream stream(&file);
            stream.setCodec("UTF-8"); // FIXME not all opmls are in utf8
            str = stream.readAll();
            file.close();
        }

        if (!doc.setContent(str))
        {

            QString backup = localFilePath()
                + QLatin1String("-backup.")
                + QString::number(QDateTime::currentDateTime().toTime_t());

            if ( QFile::copy( localFilePath(), backup ) )
                KMessageBox::error(m_mainWidget, i18n("<qt>The standard feed list is corrupted (invalid XML). A backup was created:<p><b>%1</b></p></qt>", backup), i18n("XML Parsing Error") );
            else
                KMessageBox::error(m_mainWidget, i18n("<qt>The standard feed list is corrupted (invalid XML). Could not create a backup!</p></qt>" ), i18n("XML Parsing Error") );

            if (!doc.setContent(listBackup))
                doc = createDefaultFeedList();
        }
    }

    if (!m_mainWidget->loadFeeds(doc))
    {
        QString backup = localFilePath()
            + QLatin1String("-backup.")
            + QString::number(QDateTime::currentDateTime().toTime_t());

        if ( QFile::copy( localFilePath(), backup ) )
            KMessageBox::error(m_mainWidget, i18n("<qt>The standard feed list is corrupted (no valid OPML). A backup of the previous list was created:<p><b>%1</b></p></qt>", backup), i18n("OPML Parsing Error") );
        else
            KMessageBox::error(m_mainWidget, i18n("<qt>The standard feed list is corrupted (no valid OPML). Could not create a backup!></p></qt>" ), i18n("OPML Parsing Error") );
        m_mainWidget->loadFeeds(createDefaultFeedList());
    }

    emit setStatusBarText(QString::null);	//krazy:exclude=nullstrassign for old broken gcc


    if( Settings::markAllFeedsReadOnStartup() )
        m_mainWidget->slotMarkAllFeedsRead();

    if (Settings::fetchOnStartup())
            m_mainWidget->slotFetchAllFeeds();

    return true;
}

bool Part::writeToTextFile( const QString& data, const QString& filename ) const {
    KSaveFile file( filename );
    if ( !file.open( QIODevice::WriteOnly ) )
        return false;
    QTextStream stream( &file );
    stream.setCodec( "UTF-8" );
    stream << data << endl;
    return file.finalize();
}


void Part::slotSaveFeedList()
{
    // don't save to the standard feed list, when it wasn't completely loaded before
    if ( !m_standardListLoaded )
        return;

    // the first time we overwrite the feed list, we create a backup
    if ( !m_backedUpList )
    {
        const QString backup = localFilePath() + QLatin1String( "~" );
        if ( QFile::copy( localFilePath(), backup ) )
            m_backedUpList = true;
    }

    const QString xml = m_mainWidget->feedListToOPML().toString();
    m_storage->storeFeedList( xml );
    if ( writeToTextFile( xml, localFilePath() ) )
        return;

    KMessageBox::error( m_mainWidget,
                        i18n( "Access denied: Cannot save feed list to <b>%1</b>. Please check your permissions.", localFilePath() ),
                        i18n( "Write error" ) );
}

bool Part::isTrayIconEnabled() const
{
    return Settings::showTrayIcon();
}

QWidget* Part::getMainWindow()
{
    // this is a dirty fix to get the main window used for the tray icon
    QWidgetList l = QApplication::topLevelWidgets();
    QWidget* wid = 0L;

    // check if there is an akregator main window
    foreach (wid, QApplication::topLevelWidgets())
    {
        if (wid->objectName() == "akregator_mainwindow")
            return wid;
    }

    // if not, check if there is an kontact main window
    foreach (wid, QApplication::topLevelWidgets())
    {
        if (wid->objectName().startsWith("kontact-mainwindow"))
            return wid;
    }

    return 0;
}

void Part::importFile(const KUrl& url)
{
    QString filename;

    bool isRemote = false;

    if (url.isLocalFile())
        filename = url.path();
    else
    {
        isRemote = true;

        if (!KIO::NetAccess::download(url, filename, m_mainWidget) )
        {
            KMessageBox::error(m_mainWidget, KIO::NetAccess::lastErrorString() );
            return;
        }
    }

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly))
    {
        // Read OPML feeds list and build QDom tree.
        QDomDocument doc;
        if (doc.setContent(file.readAll()))
            m_mainWidget->importFeeds(doc);
        else
            KMessageBox::error(m_mainWidget, i18n("Could not import the file %1 (no valid OPML)", filename), i18n("OPML Parsing Error") );
    }
    else
        KMessageBox::error(m_mainWidget, i18n("The file %1 could not be read, check if it exists or if it is readable for the current user.", filename), i18n("Read Error"));

    if (isRemote)
        KIO::NetAccess::removeTempFile(filename);
}

void Part::exportFile(const KUrl& url)
{
    if (url.isLocalFile())
    {
        const QString fname = url.path();

        if ( QFile::exists( fname ) &&
                KMessageBox::questionYesNo(m_mainWidget,
            i18n("The file %1 already exists; do you want to overwrite it?", fname ),
            i18n("Export"),
            KStandardGuiItem::overwrite(),
            KStandardGuiItem::cancel()) == KMessageBox::No )
            return;

        if ( !writeToTextFile( m_mainWidget->feedListToOPML().toString(), fname ) )
            KMessageBox::error(m_mainWidget, i18n("Access denied: cannot write to file %1. Please check your permissions.", fname), i18n("Write Error") );

        return;
    }
    else
    {
        KTemporaryFile tmpfile;
        tmpfile.open();

        QTextStream stream(&tmpfile);
        stream.setCodec("UTF-8");

        stream << m_mainWidget->feedListToOPML().toString() << "\n";
        stream.flush();

        if (!KIO::NetAccess::upload(tmpfile.fileName(), url, m_mainWidget))
            KMessageBox::error(m_mainWidget, KIO::NetAccess::lastErrorString() );
    }
}

void Part::fileImport()
{
    KUrl url = KFileDialog::getOpenUrl( KUrl(),
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );

    if (!url.isEmpty())
        importFile(url);
}

    void Part::fileExport()
{
    KUrl url= KFileDialog::getSaveUrl( KUrl(),
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );

    if ( !url.isEmpty() )
        exportFile(url);
}

void Part::fetchAllFeeds()
{
    m_mainWidget->slotFetchAllFeeds();
}

void Part::fetchFeedUrl(const QString&s)
{
    kDebug() <<"fetchFeedURL==" << s;
}

void Part::addFeedsToGroup(const QStringList& urls, const QString& group)
{
    for (QStringList::ConstIterator it = urls.begin(); it != urls.end(); ++it)
    {
        kDebug() <<"Akregator::Part::addFeedToGroup adding feed with URL" << *it <<" to group" << group;
        m_mainWidget->addFeedToGroup(*it, group);
    }
    NotificationManager::self()->slotNotifyFeeds(urls);
}

void Part::addFeed()
{
    m_mainWidget->slotFeedAdd();
}

void Part::showKNotifyOptions()
{
    KAboutData* about = new Akregator::AboutData;
    KNotifyConfigWidget::configure(m_mainWidget, about->appName() );
    delete about;
}

void Part::showOptions()
{
    saveSettings();

    if ( !m_dialog ) {
        m_dialog = new KCMultiDialog( m_mainWidget );
        connect( m_dialog, SIGNAL(configCommitted()),
                 this, SLOT(slotSettingsChanged()) );
        connect( m_dialog, SIGNAL(configCommitted()),
                 TrayIcon::getInstance(), SLOT(settingsChanged()) );

        QStringList modules;

        modules.append( "akregator_config_general.desktop" );
        modules.append( "akregator_config_onlinesync.desktop" );
        modules.append( "akregator_config_archive.desktop" );
        modules.append( "akregator_config_appearance.desktop" );
        modules.append( "akregator_config_browser.desktop" );
        modules.append( "akregator_config_advanced.desktop" );

        // add them all
        QStringList::iterator mit;
        for ( mit = modules.begin(); mit != modules.end(); ++mit ) {
            m_dialog->addModule( *mit );
        }
    }

    m_dialog->show();
    m_dialog->raise();
}

KParts::Part* Part::hitTest(QWidget *widget, const QPoint &globalPos)
{
/*    bool child = false;
    QWidget *me = this->widget();
    while (widget)
    {
        if (widget == me)
        {
            child = true;
            break;
        }
        if (!widget)
        {
            break;
        }
        widget = widget->parentWidget();
    }
    if (m_mainWidget && m_mainWidget->currentFrame() && child)
    {
        return m_mainWidget->currentFrame()->part();
    }
    else
    {*/
        return inherited::hitTest(widget, globalPos);
/*    }*/
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

    KConfigGroup conf( Settings::self()->config(), "HTML Settings");

    KConfig _konq( "konquerorrc", KConfig::NoGlobals  );
    KConfigGroup konq(&_konq, "HTML Settings");

    if (!conf.hasKey("MinimumFontSize"))
    {
        int minfs;
        if (konq.hasKey("MinimumFontSize"))
            minfs = konq.readEntry("MinimumFontSize", 0);
        else
            minfs = KGlobalSettings::generalFont().pointSize();
        kDebug() <<"Part::initFonts(): set MinimumFontSize to" << minfs;
        Settings::setMinimumFontSize(minfs);
    }

    if (!conf.hasKey("MediumFontSize"))
    {
        int medfs;
        if (konq.hasKey("MediumFontSize"))
            medfs = konq.readEntry("MediumFontSize", 0);
        else
            medfs = KGlobalSettings::generalFont().pointSize();
        kDebug() <<"Part::initFonts(): set MediumFontSize to" << medfs;
        Settings::setMediumFontSize(medfs);
    }

    if (!conf.hasKey("UnderlineLinks"))
    {
        bool underline = true;
        if (konq.hasKey("UnderlineLinks"))
            underline = konq.readEntry("UnderlineLinks", false);

        kDebug() <<"Part::initFonts(): set UnderlineLinks to" << underline;
        Settings::setUnderlineLinks(underline);
    }

}

} // namespace Akregator

#include "akregator_part.moc"
