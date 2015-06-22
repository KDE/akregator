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

#include "akregator_part.h"
#include "akregator_debug.h"
#include "akregatorconfig.h"
#include "aboutdata.h"
#include "actionmanagerimpl.h"
#include "article.h"
#include "fetchqueue.h"
#include "feediconmanager.h"
#include "framemanager.h"
#include "kernel.h"
#include "loadfeedlistcommand.h"
#include "mainwidget.h"
#include "notificationmanager.h"
#include "plugin.h"
#include "pluginmanager.h"
#include "storage.h"
#include "storagefactory.h"
#include "storagefactoryregistry.h"
#include "trayicon.h"
#include "dummystorage/storagefactorydummyimpl.h"
#include "utils.h"
#include "akregator_options.h"
#include <libkdepim/misc/broadcaststatus.h>
#include "kdepim-version.h"

#include <knotifyconfigwidget.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <qfiledialog.h>
#include <kmessagebox.h>

#include <QTemporaryFile>
#include <QSaveFile>
#include <kservice.h>
#include <kxmlguifactory.h>
#include <kio/netaccess.h>
#include <KPluginFactory>
#include <KParts/Plugin>
#include <KCMultiDialog>
#include <kstandardaction.h>

#include <QApplication>
#include <QFile>
#include <QObject>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#include <QDomDocument>
#include "akregratormigrateapplication.h"
#include "partadaptor.h"

#include <memory>
#include <QFontDatabase>
#include <QStandardPaths>

namespace
{

static QDomDocument createDefaultFeedList()
{
    QDomDocument doc;
    QDomProcessingInstruction z = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(z);

    QDomElement root = doc.createElement("opml");
    root.setAttribute("version", "1.0");
    doc.appendChild(root);

    QDomElement head = doc.createElement("head");
    root.appendChild(head);

    QDomElement text = doc.createElement("text");
    text.appendChild(doc.createTextNode(i18n("Feeds")));
    head.appendChild(text);

    QDomElement body = doc.createElement("body");
    root.appendChild(body);

    QDomElement mainFolder = doc.createElement("outline");
    mainFolder.setAttribute("text", "KDE");
    body.appendChild(mainFolder);

    /*
    // NOTE: If these feeds ever get updated again, reenable them.
    //       For now I (Bertjan, July 2010) just disable them to
    //       make the default feeds non-embarassing (these are
    //       still updated regularly.
    QDomElement ak = doc.createElement( "outline" );
    ak.setAttribute("text",i18n("Akregator News"));
    ak.setAttribute("xmlUrl","http://akregator.sf.net/rss2.php");
    mainFolder.appendChild(ak);

    QDomElement akb = doc.createElement( "outline" );
    akb.setAttribute("text",i18n("Akregator Blog"));
    akb.setAttribute("xmlUrl","http://akregator.pwsp.net/blog/?feed=rss2");
    mainFolder.appendChild(akb);
    */

    QDomElement dot = doc.createElement("outline");
    dot.setAttribute("text", i18n("KDE Dot News"));
    dot.setAttribute("xmlUrl", "http://www.kde.org/dotkdeorg.rdf");
    mainFolder.appendChild(dot);

    QDomElement linuxFeeds = doc.createElement("outline");
    linuxFeeds.setAttribute("text", i18n("Linux.com"));
    linuxFeeds.setAttribute("xmlUrl", "https://www.linux.com/rss/feeds.php");
    mainFolder.appendChild(linuxFeeds);

    QDomElement planetkde = doc.createElement("outline");
    planetkde.setAttribute("text", i18n("Planet KDE"));
    planetkde.setAttribute("xmlUrl", "http://planetkde.org/rss20.xml");
    mainFolder.appendChild(planetkde);

    QDomElement planetkdepim = doc.createElement("outline");
    planetkdepim.setAttribute("text", i18n("Planet KDE PIM"));
    planetkdepim.setAttribute("xmlUrl", "http://pim.planetkde.org/rss20.xml");
    mainFolder.appendChild(planetkdepim);

    QDomElement apps = doc.createElement("outline");
    apps.setAttribute("text", i18n("KDE Apps"));
    apps.setAttribute("xmlUrl", "http://www.kde.org/dot/kde-apps-content.rdf");
    mainFolder.appendChild(apps);

    QDomElement look = doc.createElement("outline");
    look.setAttribute("text", i18n("KDE Look"));
    look.setAttribute("xmlUrl", "http://www.kde.org/kde-look-content.rdf");
    mainFolder.appendChild(look);

    // hungarian feed(s)
    QDomElement hungarianFolder = doc.createElement("outline");
    hungarianFolder.setAttribute("text", i18n("Hungarian feeds"));
    mainFolder.appendChild(hungarianFolder);

    QDomElement hungarianKde = doc.createElement("outline");
    hungarianKde.setAttribute("text", i18n("KDE.HU"));
    hungarianKde.setAttribute("xmlUrl", "http://kde.hu/rss.xml");
    hungarianFolder.appendChild(hungarianKde);

    // spanish feed(s)
    QDomElement spanishFolder = doc.createElement("outline");
    spanishFolder.setAttribute("text", i18n("Spanish feeds"));
    mainFolder.appendChild(spanishFolder);

    QDomElement spanishKde = doc.createElement("outline");
    spanishKde.setAttribute("text", i18n("Planet KDE España"));
    spanishKde.setAttribute("xmlUrl", "http://planet.kde-espana.es/");
    spanishFolder.appendChild(spanishKde);

    return doc;
}
}

namespace Akregator
{

K_PLUGIN_FACTORY(AkregatorFactory, registerPlugin<Part>();)

BrowserExtension::BrowserExtension(Part *p, const char *name)
    : KParts::BrowserExtension(p)
{
    AkregratorMigrateApplication migrate;
    migrate.migrate();
    setObjectName(name);
    m_part = p;
}

void BrowserExtension::saveSettings()
{
    m_part->saveSettings();
}

Part::Part(QWidget *parentWidget, QObject *parent, const QVariantList &)
    : inherited(parent)
    , m_standardListLoaded(false)
    , m_shuttingDown(false)
    , m_doCrashSave(true)
    , m_backedUpList(false)
    , m_mainWidget(0)
    , m_storage(0)
    , m_dialog(0)

{
    initFonts();

    setPluginLoadingMode(LoadPluginsIfEnabled);
    setPluginInterfaceVersion(AKREGATOR_PLUGIN_INTERFACE_VERSION);

    setComponentName(QStringLiteral("akregator"), QStringLiteral("akregator"));
    setXMLFile("akregator_part.rc", true);

    new PartAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/Akregator", this);

    FeedIconManager::self(); // FIXME: registering the icon manager dbus iface here,
    // because otherwise we get a deadlock later

    m_standardFeedList = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/akregator/data/feeds.opml");
    QFileInfo fileInfo(m_standardFeedList);
    QDir().mkpath(fileInfo.absolutePath());

    Backend::StorageFactoryDummyImpl *dummyFactory = new Backend::StorageFactoryDummyImpl();
    if (!Backend::StorageFactoryRegistry::self()->registerFactory(dummyFactory, dummyFactory->key())) {
        // There was already a dummy factory registered.
        delete dummyFactory;
    }
    loadPlugins(QStringLiteral("storage"));   // FIXME: also unload them!

    m_storage = 0;
    Backend::StorageFactory *storageFactory = Backend::StorageFactoryRegistry::self()->getFactory(Settings::archiveBackend());
    if (storageFactory != 0) {
        m_storage = storageFactory->createStorage(QStringList());
    }

    if (!m_storage) { // Houston, we have a problem
        m_storage = Backend::StorageFactoryRegistry::self()->getFactory("dummy")->createStorage(QStringList());

        KMessageBox::error(parentWidget, i18n("Unable to load storage backend plugin \"%1\". No feeds are archived.", Settings::archiveBackend()), i18n("Plugin error"));
    }

    m_storage->open(true);
    Kernel::self()->setStorage(m_storage);

    m_actionManager = new ActionManagerImpl(this);
    ActionManager::setInstance(m_actionManager);

    m_mainWidget = new Akregator::MainWidget(this, parentWidget, m_actionManager, "akregator_view");
    m_extension = new BrowserExtension(this, "ak_extension");

    connect(Kernel::self()->frameManager(), &FrameManager::signalCaptionChanged, this, &Part::setWindowCaption);
    connect(Kernel::self()->frameManager(), &FrameManager::signalStatusText, this, &Part::slotSetStatusText);
    connect(Kernel::self()->frameManager(), &FrameManager::signalLoadingProgress, m_extension, &BrowserExtension::loadingProgress);
    connect(Kernel::self()->frameManager(), &FrameManager::signalCanceled, this, &ReadOnlyPart::canceled);
    connect(Kernel::self()->frameManager(), &FrameManager::signalStarted, this, &Part::slotStarted);
    connect(Kernel::self()->frameManager(), SIGNAL(signalCompleted()), this, SIGNAL(completed()));

    // notify the part that this is our internal widget
    setWidget(m_mainWidget);

    if (Settings::showTrayIcon() && !TrayIcon::getInstance()) {
        TrayIcon *trayIcon = new TrayIcon(m_mainWidget->window());
        TrayIcon::setInstance(trayIcon);
        m_actionManager->setTrayIcon(trayIcon);

        if (isTrayIconEnabled()) {
            trayIcon->setStatus(KStatusNotifierItem::Active);
        }

        QWidget *const notificationParent = isTrayIconEnabled() ? m_mainWidget->window() : 0;
        NotificationManager::self()->setWidget(notificationParent, componentData().componentName());

        connect(m_mainWidget, SIGNAL(signalUnreadCountChanged(int)), trayIcon, SLOT(slotSetUnread(int)));
        connect(m_mainWidget, &MainWidget::signalArticlesSelected,
                this, &Part::signalArticlesSelected);
    }

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotOnShutdown()));

    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, &QTimer::timeout, this, &Part::slotSaveFeedList);
    m_autosaveTimer->start(5 * 60 * 1000); // 5 minutes

    QString useragent = QString("Akregator/%1; syndication").arg(KDEPIM_VERSION);

    if (!Settings::customUserAgent().isEmpty()) {
        useragent = Settings::customUserAgent();
    }

    Syndication::FileRetriever::setUserAgent(useragent);

    loadPlugins(QStringLiteral("extension"));   // FIXME: also unload them!

    if (!readCrashProperties()) {
        autoReadProperties();
    }
}

void Part::loadPlugins(const QString &type)
{
    const KService::List offers = PluginManager::query(QStringLiteral("[X-KDE-akregator-plugintype] == '%1'").arg(type));

    Q_FOREACH (const KService::Ptr &i, offers) {
        Akregator::Plugin *plugin = PluginManager::createFromService(i, this);
        if (!plugin) {
            continue;
        }
        plugin->initialize();
        plugin->insertGuiClients(this);
    }
}

void Part::slotStarted()
{
    Q_EMIT started(Q_NULLPTR);
}

void Part::slotOnShutdown()
{
    autoSaveProperties();
    m_shuttingDown = true;
    m_autosaveTimer->stop();
    saveSettings();
    slotSaveFeedList();
    m_mainWidget->slotOnShutdown();
    //delete m_mainWidget;
    delete TrayIcon::getInstance();
    TrayIcon::setInstance(Q_NULLPTR);
    delete m_storage;
    m_storage = 0;
    //delete m_actionManager;
}

void Part::slotSettingsChanged()
{
    //QT5 NotificationManager::self()->setWidget(isTrayIconEnabled() ? m_mainWidget->window() : 0, componentData());

    if (Settings::showTrayIcon() && !TrayIcon::getInstance()) {
        TrayIcon *trayIcon = new TrayIcon(m_mainWidget->window());
        TrayIcon::setInstance(trayIcon);
        m_actionManager->setTrayIcon(trayIcon);

        if (isTrayIconEnabled()) {
            trayIcon->setStatus(KStatusNotifierItem::Active);
        }

        connect(m_mainWidget, SIGNAL(signalUnreadCountChanged(int)), trayIcon, SLOT(slotSetUnread(int)));
        connect(m_mainWidget, &MainWidget::signalArticlesSelected,
                this, &Part::signalArticlesSelected);

        m_mainWidget->slotSetTotalUnread();
    }
    if (!Settings::showTrayIcon()) {
        TrayIcon::getInstance()->disconnect();
        delete TrayIcon::getInstance();
        TrayIcon::setInstance(0);
        m_actionManager->setTrayIcon(0);
    }

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

    if (Settings::minimumFontSize() > Settings::mediumFontSize()) {
        Settings::setMediumFontSize(Settings::minimumFontSize());
    }
    saveSettings();
    Q_EMIT signalSettingsChanged();
}

void Part::slotSetStatusText(const QString &statusText)
{
    KPIM::BroadcastStatus::instance()->setStatusMsg(statusText);
}

void Part::saveSettings()
{
    m_mainWidget->saveSettings();
}

Part::~Part()
{
    qCDebug(AKREGATOR_LOG) << "Part::~Part() enter";
    if (!m_shuttingDown) {
        slotOnShutdown();
    }
    delete m_dialog;
    qCDebug(AKREGATOR_LOG) << "Part::~Part(): leaving";
}

void Part::readProperties(const KConfigGroup &config)
{
    m_backedUpList = false;
    openStandardFeedList();

    if (m_mainWidget) {
        m_mainWidget->readProperties(config);
    }
}

void Part::saveProperties(KConfigGroup &config)
{
    if (m_mainWidget) {
        slotSaveFeedList();
        m_mainWidget->saveProperties(config);
    }
}

bool Part::openUrl(const QUrl &url)
{
    setLocalFilePath(url.toLocalFile());
    return openFile();
}

void Part::openStandardFeedList()
{
    if (!m_standardFeedList.isEmpty()) {
        openUrl(QUrl::fromLocalFile(m_standardFeedList));
    }
}

bool Part::openFile()
{
    if (m_loadFeedListCommand || m_standardListLoaded) {
        return true;
    }
    QScopedPointer<LoadFeedListCommand> cmd(new LoadFeedListCommand(m_mainWidget));
    cmd->setParentWidget(m_mainWidget);
    cmd->setStorage(Kernel::self()->storage());
    cmd->setFileName(localFilePath());
    cmd->setDefaultFeedList(createDefaultFeedList());
    connect(cmd.data(), SIGNAL(result(QSharedPointer<Akregator::FeedList>)),
            this, SLOT(feedListLoaded(QSharedPointer<Akregator::FeedList>)));
    m_loadFeedListCommand = cmd.take();
    m_loadFeedListCommand->start();
    return true;
}

bool Part::writeToTextFile(const QString &data, const QString &filename) const
{
    QSaveFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << data << endl;
    return file.commit();
}

void Part::feedListLoaded(const QSharedPointer<FeedList> &list)
{
    Q_ASSERT(!m_standardListLoaded);
    m_mainWidget->setFeedList(list);
    m_standardListLoaded = list != 0;

    if (Settings::markAllFeedsReadOnStartup()) {
        m_mainWidget->slotMarkAllFeedsRead();
    }

    if (m_standardListLoaded) {
        QTimer::singleShot(0, this, SLOT(flushAddFeedRequests()));
    }

    if (Settings::fetchOnStartup()) {
        m_mainWidget->slotFetchAllFeeds();
    }
}

void Part::flushAddFeedRequests()
{
    Q_FOREACH (const AddFeedRequest &i, m_requests) {
        Q_FOREACH (const QString &j, i.urls) {
            m_mainWidget->addFeedToGroup(j, i.group);
        }
        NotificationManager::self()->slotNotifyFeeds(i.urls);
    }
    m_requests.clear();
}

void Part::slotSaveFeedList()
{
    // don't save to the standard feed list, when it wasn't completely loaded before
    if (!m_standardListLoaded) {
        return;
    }

    // the first time we overwrite the feed list, we create a backup
    if (!m_backedUpList) {
        const QString backup = localFilePath() + QStringLiteral("~");
        if (QFile::copy(localFilePath(), backup)) {
            m_backedUpList = true;
        }
    }

    const QString xml = m_mainWidget->feedListToOPML().toString();
    m_storage->storeFeedList(xml);
    if (writeToTextFile(xml, localFilePath())) {
        return;
    }

    KMessageBox::error(m_mainWidget,
                       i18n("Access denied: Cannot save feed list to <b>%1</b>. Please check your permissions.", localFilePath()),
                       i18n("Write Error"));
}

bool Part::isTrayIconEnabled() const
{
    return Settings::showTrayIcon();
}

void Part::importFile(const QUrl &url)
{
    QString filename;

    bool isRemote = false;

    if (url.isLocalFile()) {
        filename = url.toLocalFile();
    } else {
        isRemote = true;

        if (!KIO::NetAccess::download(url, filename, m_mainWidget)) {
            KMessageBox::error(m_mainWidget, KIO::NetAccess::lastErrorString());
            return;
        }
    }

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        // Read OPML feeds list and build QDom tree.
        QDomDocument doc;
        if (doc.setContent(file.readAll())) {
            m_mainWidget->importFeedList(doc);
        } else {
            KMessageBox::error(m_mainWidget, i18n("Could not import the file %1 (no valid OPML)", filename), i18n("OPML Parsing Error"));
        }
    } else {
        KMessageBox::error(m_mainWidget, i18n("The file %1 could not be read, check if it exists or if it is readable for the current user.", filename), i18n("Read Error"));
    }

    if (isRemote) {
        KIO::NetAccess::removeTempFile(filename);
    }
}

void Part::exportFile(const QUrl &url)
{
    if (url.isLocalFile()) {
        const QString fname = url.toLocalFile();

        if (QFile::exists(fname) &&
                KMessageBox::questionYesNo(m_mainWidget,
                                           i18n("The file %1 already exists; do you want to overwrite it?", fname),
                                           i18n("Export"),
                                           KStandardGuiItem::overwrite(),
                                           KStandardGuiItem::cancel()) == KMessageBox::No) {
            return;
        }

        if (!writeToTextFile(m_mainWidget->feedListToOPML().toString(), fname)) {
            KMessageBox::error(m_mainWidget, i18n("Access denied: cannot write to file %1. Please check your permissions.", fname), i18n("Write Error"));
        }

        return;
    } else {
        QTemporaryFile tmpfile;
        tmpfile.open();

        QTextStream stream(&tmpfile);
        stream.setCodec("UTF-8");

        stream << m_mainWidget->feedListToOPML().toString() << "\n";
        stream.flush();

        if (!KIO::NetAccess::upload(tmpfile.fileName(), url, m_mainWidget)) {
            KMessageBox::error(m_mainWidget, KIO::NetAccess::lastErrorString());
        }
    }
}

void Part::fileImport()
{
    const QString filters = i18n("OPML Outlines (*.opml, *.xml);;All Files (*)");
    const QUrl url = QFileDialog::getOpenFileUrl(m_mainWidget, QString(), QUrl(), filters);
    if (!url.isEmpty()) {
        importFile(url);
    }
}

void Part::fileExport()
{
    const QString filters = i18n("OPML Outlines (*.opml, *.xml);;All Files (*)");
    const QUrl url = QFileDialog::getSaveFileUrl(m_mainWidget, QString(), QUrl(), filters);

    if (!url.isEmpty()) {
        exportFile(url);
    }
}

void Part::fetchAllFeeds()
{
    m_mainWidget->slotFetchAllFeeds();
}

void Part::fetchFeedUrl(const QString &s)
{
    qCDebug(AKREGATOR_LOG) << "fetchFeedURL==" << s;
}

void Part::addFeedsToGroup(const QStringList &urls, const QString &group)
{
    AddFeedRequest req;
    req.group = group;
    req.urls = urls;
    m_requests.append(req);
    if (m_standardListLoaded) {
        flushAddFeedRequests();
    }
}

void Part::addFeed()
{
    m_mainWidget->slotFeedAdd();
}

void Part::showNotificationOptions()
{
    const Akregator::AboutData about;
    KNotifyConfigWidget::configure(m_mainWidget, about.productName());
}

void Part::showOptions()
{
    saveSettings();

    if (!m_dialog) {
        m_dialog = new KCMultiDialog(m_mainWidget);
        connect(m_dialog, SIGNAL(configCommitted()),
                this, SLOT(slotSettingsChanged()));
        connect(m_dialog, SIGNAL(configCommitted()),
                TrayIcon::getInstance(), SLOT(settingsChanged()));

        // query for akregator's kcm modules
        const QString constraint = "[X-KDE-ParentApp] == 'akregator'";
        const KService::List offers = KServiceTypeTrader::self()->query("KCModule", constraint);
        foreach (const KService::Ptr &service, offers) {
            m_dialog->addModule(service->storageId());
        }
    }

    m_dialog->show();
    m_dialog->raise();
}

KParts::Part *Part::hitTest(QWidget *widget, const QPoint &globalPos)
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
    if (fonts.isEmpty()) {
        fonts.append(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family());
        fonts.append(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
        fonts.append(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family());
        fonts.append(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family());
        fonts.append("0");
    }
    Settings::setFonts(fonts);
    if (Settings::standardFont().isEmpty()) {
        Settings::setStandardFont(fonts[0]);
    }
    if (Settings::fixedFont().isEmpty()) {
        Settings::setFixedFont(fonts[1]);
    }
    if (Settings::sansSerifFont().isEmpty()) {
        Settings::setSansSerifFont(fonts[2]);
    }
    if (Settings::serifFont().isEmpty()) {
        Settings::setSerifFont(fonts[3]);
    }

    KConfigGroup conf(Settings::self()->config(), "HTML Settings");

    KConfig _konq("konquerorrc", KConfig::NoGlobals);
    KConfigGroup konq(&_konq, "HTML Settings");

    if (!conf.hasKey("MinimumFontSize")) {
        int minfs;
        if (konq.hasKey("MinimumFontSize")) {
            minfs = konq.readEntry("MinimumFontSize", 8);
        } else {
            minfs = std::max(QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize() - 2, 4);
        }
        Settings::setMinimumFontSize(minfs);
    }

    if (!conf.hasKey("MediumFontSize")) {
        int medfs;
        if (konq.hasKey("MediumFontSize")) {
            medfs = konq.readEntry("MediumFontSize", 12);
        } else {
            medfs = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();
        }
        Settings::setMediumFontSize(medfs);
    }

    if (!conf.hasKey("UnderlineLinks")) {
        bool underline = true;
        if (konq.hasKey("UnderlineLinks")) {
            underline = konq.readEntry("UnderlineLinks", false);
        }
        Settings::setUnderlineLinks(underline);
    }
}

bool Part::handleCommandLine(const QStringList &args)
{
    QCommandLineParser parser;
    akregator_options(&parser);
    parser.process(args);

    const QString addFeedGroup = parser.isSet(QStringLiteral("group"))
                                 ? parser.value(QStringLiteral("group"))
                                 : i18n("Imported Folder");

    QStringList feedsToAdd = parser.values(QStringLiteral("addfeed"));

    if (feedsToAdd.isEmpty() && !parser.positionalArguments().isEmpty()) {
        for (const QString &url : parser.positionalArguments()) {
            feedsToAdd.append(url);
        }
    }

    if (!feedsToAdd.isEmpty()) {
        addFeedsToGroup(feedsToAdd, addFeedGroup);
    }
    return true;
}

void Part::clearCrashProperties()
{
    if (!m_doCrashSave) {
        return;
    }
    KConfig config("crashed", KConfig::SimpleConfig,
                   QStandardPaths::ApplicationsLocation);
    KConfigGroup configGroup(&config, "Part");
    configGroup.writeEntry("crashed", false);
}

void Part::saveCrashProperties()
{
    if (!m_doCrashSave) {
        return;
    }
    KConfig config("crashed", KConfig::SimpleConfig,
                   QStandardPaths::ApplicationsLocation);
    KConfigGroup configGroup(&config, "Part");
    configGroup.deleteGroup();

    configGroup.writeEntry("crashed", true);

    saveProperties(configGroup);
}

bool Part::readCrashProperties()
{
    KConfig config("crashed", KConfig::SimpleConfig,
                   QStandardPaths::ApplicationsLocation);
    KConfigGroup configGroup(&config, "Part");

    if (!configGroup.readEntry("crashed", false)) {
        return false;
    }

    const int choice = KMessageBox::questionYesNoCancel(m_mainWidget,
                       i18n("Akregator did not close correctly. Would you like to restore the previous session?"),
                       i18n("Restore Session?"),
                       KGuiItem(i18n("Restore Session"), "window-new"),
                       KGuiItem(i18n("Do Not Restore"), "dialog-close"),
                       KGuiItem(i18n("Ask Me Later"), "chronometer"),
                       "Restore session when akregator didn't close correctly");
    switch (choice) {
    case KMessageBox::Yes:
        readProperties(configGroup);
        clearCrashProperties();
        return true;
    case KMessageBox::No:
        clearCrashProperties();
        return false;
    default:
        break;
    }
    m_doCrashSave = false;
    return false;
}

void Part::slotAutoSave()
{
    saveCrashProperties();
}

void Part::autoSaveProperties()
{
    KConfig config("autosaved", KConfig::SimpleConfig, QStandardPaths::ApplicationsLocation);
    KConfigGroup configGroup(&config, "Part");
    configGroup.deleteGroup();

    saveProperties(configGroup);

    clearCrashProperties();
}

void Part::autoReadProperties()
{
    if (qGuiApp->isSessionRestored()) {
        return;
    }
    KConfig config("autosaved", KConfig::SimpleConfig, QStandardPaths::ApplicationsLocation);
    KConfigGroup configGroup(&config, "Part");

    readProperties(configGroup);
}

} // namespace Akregator
#include "akregator_part.moc"
