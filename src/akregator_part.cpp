/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
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
#include <dcopclient.h>
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
#include <knotifyclient.h>
#include <knotifydialog.h>
#include <kmenu.h>
#include <kservice.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <ktempfile.h>
#include <ktrader.h>
#include <ktoolinvocation.h>
#include <kxmlguifactory.h>
#include <kio/netaccess.h>
#include <kparts/browserinterface.h>
#include <kparts/genericfactory.h>
#include <kparts/partmanager.h>

#include <qfile.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qwidget.h>
#include <QTextStream>
#include <ktoolinvocation.h>

#include "aboutdata.h"
#include "actionmanagerimpl.h"
#include "akregator_part.h"
#include "akregator_view.h"
#include "akregatorconfig.h"
#include "articlefilter.h"
#include "articleinterceptor.h"
#include "configdialog.h"
#include "fetchqueue.h"
#include "frame.h"
#include "article.h"
#include "kernel.h"
#include "notificationmanager.h"
#include "pageviewer.h"
#include "plugin.h"
#include "pluginmanager.h"
#include "storage.h"
#include "storagefactory.h"
#include "storagefactorydummyimpl.h"
#include "storagefactoryregistry.h"
#include "speechclient.h"
#include "trayicon.h"
#include "tagset.h"
#include "tag.h"

namespace Akregator {

typedef KParts::GenericFactory<Part> AkregatorFactory;
K_EXPORT_COMPONENT_FACTORY( libakregatorpart, AkregatorFactory )

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

class Part::ApplyFiltersInterceptor : public ArticleInterceptor
{
    public:
    virtual void processArticle(Article& article)
    {
        Filters::ArticleFilterList list = Kernel::self()->articleFilterList();
        for (Filters::ArticleFilterList::ConstIterator it = list.begin(); it != list.end(); ++it)
            (*it).applyTo(article);
    }
};

Part::Part( QWidget *parentWidget, const char * /*widgetName*/,
                              QObject *parent, const char *name, const QStringList& )
    : DCOPObject("AkregatorIface")
       , MyBasePart(parent)
       , m_standardListLoaded(false)
       , m_shuttingDown(false)
       , m_mergedPart(0)
       , m_backedUpList(false)
       , m_storage(0)
{
    setObjectName(name);
    // we need an instance
    setInstance( AkregatorFactory::instance() );

    // start knotifyclient if not already started. makes it work for people who doesn't use full kde, according to kmail devels
    KNotifyClient::startDaemon();

    m_standardFeedList = KGlobal::dirs()->saveLocation("data", "akregator/data") + "/feeds.opml";

    m_tagSetPath = KGlobal::dirs()->saveLocation("data", "akregator/data") + "/tagset.xml";

    Backend::StorageFactoryDummyImpl* dummyFactory = new Backend::StorageFactoryDummyImpl();
    Backend::StorageFactoryRegistry::self()->registerFactory(dummyFactory, dummyFactory->key());
    loadPlugins(); // FIXME: also unload them!

    m_storage = 0;
    Backend::StorageFactory* factory = Backend::StorageFactoryRegistry::self()->getFactory(Settings::archiveBackend());
    if (factory != 0)
        m_storage = factory->createStorage(QStringList());

    if (!m_storage) // Houston, we have a problem
    {
        m_storage = Backend::StorageFactoryRegistry::self()->getFactory("dummy")->createStorage(QStringList());

        KMessageBox::error(m_view, i18n("Unable to load storage backend plugin \"%1\". No feeds are archived.").arg(Settings::archiveBackend()), i18n("Plugin error") );
    }

    Filters::ArticleFilterList list;
    list.readConfig(Settings::self()->config());
    Kernel::self()->setArticleFilterList(list);

    m_applyFiltersInterceptor = new ApplyFiltersInterceptor();
    ArticleInterceptorManager::self()->addInterceptor(m_applyFiltersInterceptor);

    m_storage->open(true);
    Kernel::self()->setStorage(m_storage);
    Backend::Storage::setInstance(m_storage); // TODO: kill this one

    loadTagSet(m_tagSetPath);

    m_actionManager = new ActionManagerImpl(this);
    ActionManager::setInstance(m_actionManager);

    m_view = new Akregator::View(this, parentWidget, m_actionManager, "akregator_view");
    m_actionManager->initView(m_view);
    m_actionManager->setTagSet(Kernel::self()->tagSet());

    m_extension = new BrowserExtension(this, "ak_extension");

    connect(m_view, SIGNAL(setWindowCaption(const QString&)), this, SIGNAL(setWindowCaption(const QString&)));
    connect(m_view, SIGNAL(setStatusBarText(const QString&)), this, SIGNAL(setStatusBarText(const QString&)));
    connect(m_view, SIGNAL(setProgress(int)), m_extension, SIGNAL(loadingProgress(int)));
    connect(m_view, SIGNAL(signalCanceled(const QString&)), this, SIGNAL(canceled(const QString&)));
    connect(m_view, SIGNAL(signalStarted(KIO::Job*)), this, SIGNAL(started(KIO::Job*)));
    connect(m_view, SIGNAL(signalCompleted()), this, SIGNAL(completed()));

    // notify the part that this is our internal widget
    setWidget(m_view);

    TrayIcon* trayIcon = new TrayIcon( getMainWindow() );
    TrayIcon::setInstance(trayIcon);
    m_actionManager->initTrayIcon(trayIcon);

    connect(trayIcon, SIGNAL(showPart()), this, SIGNAL(showPart()));

    if ( isTrayIconEnabled() )
    {
        trayIcon->show();
        NotificationManager::self()->setWidget(trayIcon, instance());
    }
    else
        NotificationManager::self()->setWidget(getMainWindow(), instance());

    connect( trayIcon, SIGNAL(quitSelected()),
            kapp, SLOT(quit())) ;

    connect( m_view, SIGNAL(signalUnreadCountChanged(int)), trayIcon, SLOT(slotSetUnread(int)) );

    connect(kapp, SIGNAL(aboutToQuit()), this, SLOT(slotOnShutdown()));

    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, SIGNAL(timeout()), this, SLOT(slotSaveFeedList()));
    m_autosaveTimer->start(5*60*1000); // 5 minutes

    setXMLFile("akregator_part.rc", true);

    initFonts();

    RSS::FileRetriever::setUserAgent(QString("Akregator/%1; librss/remnants").arg(AKREGATOR_VERSION));
}

void Part::loadPlugins()
{
    // "[X-KDE-akregator-plugintype] == 'storage'"
    KTrader::OfferList offers = PluginManager::query();

    for( KTrader::OfferList::ConstIterator it = offers.begin(), end = offers.end(); it != end; ++it )
    {
        Akregator::Plugin* plugin = PluginManager::createFromService(*it);
        if (plugin)
            plugin->init();
    }
}

void Part::slotOnShutdown()
{
    m_shuttingDown = true;
    m_autosaveTimer->stop();
    saveSettings();
    slotSaveFeedList();
    saveTagSet(m_tagSetPath);
    m_view->slotOnShutdown();
    //delete m_view;
    delete m_storage;
    m_storage = 0;
    //delete m_actionManager;
}

void Part::slotSettingsChanged()
{
    NotificationManager::self()->setWidget(isTrayIconEnabled() ? TrayIcon::getInstance() : getMainWindow(), instance());

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
    Kernel::self()->articleFilterList().writeConfig(Settings::self()->config());
    m_view->saveSettings();
}

Part::~Part()
{
    kdDebug() << "Part::~Part() enter" << endl;
    if (!m_shuttingDown)
        slotOnShutdown();
    kdDebug() << "Part::~Part(): leaving" << endl;
    ArticleInterceptorManager::self()->removeInterceptor(m_applyFiltersInterceptor);
    delete m_applyFiltersInterceptor;
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
    if (m_view)
    {
        slotSaveFeedList();
        m_view->saveProperties(config);
    }
}

bool Part::openURL(const KURL& url)
{
    m_file = url.path();
    return openFile();
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
    QFile file(m_file);

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
            stream.setEncoding(QTextStream::UnicodeUTF8); // FIXME not all opmls are in utf8
            str = stream.read();
            file.close();
        }

        if (!doc.setContent(str))
        {

            QString backup = m_file + "-backup." +  QString::number(QDateTime::currentDateTime().toTime_t());

            copyFile(backup);

            KMessageBox::error(m_view, i18n("<qt>The standard feed list is corrupted (invalid XML). A backup was created:<p><b>%2</b></p></qt>").arg(backup), i18n("XML Parsing Error") );

            if (!doc.setContent(listBackup))
                doc = createDefaultFeedList();
        }
    }

    if (!m_view->loadFeeds(doc))
    {
        QString backup = m_file + "-backup." +  QString::number(QDateTime::currentDateTime().toTime_t());
        copyFile(backup);

        KMessageBox::error(m_view, i18n("<qt>The standard feed list is corrupted (no valid OPML). A backup was created:<p><b>%2</b></p></qt>").arg(backup), i18n("OPML Parsing Error") );

        m_view->loadFeeds(createDefaultFeedList());
    }

    emit setStatusBarText(QString::null);
    

    if( Settings::markAllFeedsReadOnStartup() )
        m_view->slotMarkAllFeedsRead();

    if (Settings::fetchOnStartup())
            m_view->slotFetchAllFeeds();

    return true;
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

    QString xmlStr = m_view->feedListToOPML().toString();
    m_storage->storeFeedList(xmlStr);

    QFile file(m_file);
    if (file.open(QIODevice::WriteOnly) == false)
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

    stream << xmlStr << endl;

    file.close();
}

bool Part::isTrayIconEnabled() const
{
    return Settings::showTrayIcon();
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

        QWidgetList l = QApplication::topLevelWidgets();
        QListIterator<QWidget*> it( l );

        // check if there is an akregator main window
        while ( it.hasNext() )
        {
        QWidget * wid = it.next();
        //kdDebug() << "win name: " << wid->name() << endl;
        if (QString(wid->name()) == "akregator_mainwindow")
            return wid;
        }
        // if not, check for kontact main window
        it.toFront();
        while ( it.hasNext() )
        {
            QWidget * wid = it.next();
            if (QString(wid->name()).startsWith("kontact-mainwindow"))
                return wid;
        }
    return 0;
}

void Part::loadTagSet(const QString& path)
{
    QDomDocument doc;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        doc.setContent(file.readAll());
        file.close();
    }
    // if we can't load the tagset from the xml file, check for the backup in the backend
    if (doc.isNull())
    {
         doc.setContent(m_storage->restoreTagSet());
    }

    if (doc.isNull())
    {
        Kernel::self()->tagSet()->readFromXML(doc);
    }
    else
    {
        Kernel::self()->tagSet()->insert(Tag("http://akregator.sf.net/tags/Interesting", i18n("Interesting")));
    }
}

void Part::saveTagSet(const QString& path)
{

    QString xmlStr = Kernel::self()->tagSet()->toXML().toString();

    m_storage->storeTagSet(xmlStr);

    QFile file(path);

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream(&file);
        stream.setEncoding(QTextStream::UnicodeUTF8);
        stream << xmlStr << "\n";
        file.close();
    }
}

void Part::importFile(const KURL& url)
{
    QString filename;

    bool isRemote = false;

    if (url.isLocalFile())
        filename = url.path();
    else
    {
        isRemote = true;

        if (!KIO::NetAccess::download(url, filename, m_view) )
        {
            KMessageBox::error(m_view, KIO::NetAccess::lastErrorString() );
            return;
        }
    }

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly))
    {
        // Read OPML feeds list and build QDom tree.
        QDomDocument doc;
        if (doc.setContent(file.readAll()))
            m_view->importFeeds(doc);
        else
            KMessageBox::error(m_view, i18n("Could not import the file %1 (no valid OPML)").arg(filename), i18n("OPML Parsing Error") );
    }
    else
        KMessageBox::error(m_view, i18n("The file %1 could not be read, check if it exists or if it is readable for the current user.").arg(filename), i18n("Read Error"));

    if (isRemote)
        KIO::NetAccess::removeTempFile(filename);
}

void Part::exportFile(const KURL& url)
{
    if (url.isLocalFile())
    {
        QFile file(url.path());

        if ( file.exists() &&
                KMessageBox::questionYesNo(m_view,
            i18n("The file %1 already exists; do you want to overwrite it?").arg(file.name()),
            i18n("Export"),
            i18n("Overwrite"),
            KStdGuiItem::cancel()) == KMessageBox::No )
            return;

        if ( !file.open(QIODevice::WriteOnly) )
        {
            KMessageBox::error(m_view, i18n("Access denied: cannot write to file %1").arg(file.name()), i18n("Write Error") );
            return;
        }

        QTextStream stream(&file);
        stream.setEncoding(QTextStream::UnicodeUTF8);

        stream << m_view->feedListToOPML().toString() << "\n";
        file.close();
    }
    else
    {
        KTempFile tmpfile;
        tmpfile.setAutoDelete(true);

        QTextStream stream(tmpfile.file());
        stream.setEncoding(QTextStream::UnicodeUTF8);

        stream << m_view->feedListToOPML().toString() << "\n";
        tmpfile.close();

        if (!KIO::NetAccess::upload(tmpfile.name(), url, m_view))
            KMessageBox::error(m_view, KIO::NetAccess::lastErrorString() );
    }
}

void Part::fileImport()
{
    KURL url = KFileDialog::getOpenURL( QString::null,
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );

    if (!url.isEmpty())
        importFile(url);
}

    void Part::fileExport()
{
    KURL url= KFileDialog::getSaveURL( QString::null,
                        "*.opml *.xml|" + i18n("OPML Outlines (*.opml, *.xml)")
                        +"\n*|" + i18n("All Files") );

    if ( !url.isEmpty() )
        exportFile(url);
}

void Part::fileGetFeeds()
{
    /*GetFeeds *gf = new GetFeeds();
    gf->show();*/
     //KNS::DownloadDialog::open("akregator/feeds", i18n("Get New Feeds"));
}

void Part::fileSendArticle(bool attach)
{
    // FIXME: you have to open article to tab to be able to send...
    QByteArray text = m_view->currentFrame()->part()->url().prettyURL().latin1();
    if(text.isEmpty() || text.isNull())
        return;

    QString title = m_view->currentFrame()->title();

    if(attach) {
        KToolInvocation::invokeMailer(QString(),
                           QString(),
                           QString(),
                           title,
                           text,
                           QString(),
                           QStringList(),
                           text);
    }
    else {
        KToolInvocation::invokeMailer(QString(),
                           QString(),
                           QString(),
                           QString(),
                           title,
                           QString(),
                           QStringList(),
                           text);
    }
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

void Part::addFeed()
{
    m_view->slotFeedAdd();
}

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

    KConfigDialog* dialog = new ConfigDialog( m_view, "settings", Settings::self() );

    connect( dialog, SIGNAL(settingsChanged()),
             this, SLOT(slotSettingsChanged()) );
    connect( dialog, SIGNAL(settingsChanged()),
             TrayIcon::getInstance(), SLOT(settingsChanged()) );

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

    if (file.open(QIODevice::ReadOnly))
    {
        QFile backupFile(backup);
        if (backupFile.open(QIODevice::WriteOnly))
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
