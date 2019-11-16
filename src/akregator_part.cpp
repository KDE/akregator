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

#include "messageviewer/messageviewersettings.h"
#include "akregatorconfig.h"
#include "aboutdata.h"
#include "actionmanagerimpl.h"
#include "article.h"
#include "fetchqueue.h"
#include "feedlist.h"
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
#include "widgets/akregatorcentralwidget.h"
#include "dummystorage/storagefactorydummyimpl.h"
#include "utils.h"
#include "akregator_options.h"
#include <Libkdepim/BroadcastStatus>
#include "akregator-version.h"
#include "unityservicemanager.h"
#include <kio/filecopyjob.h>
#include <knotifyconfigwidget.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <QFileDialog>
#include <kmessagebox.h>

#include <QTemporaryFile>
#include <QWebEngineSettings>
#include <QSaveFile>
#include <kservice.h>
#include <kxmlguifactory.h>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
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

namespace {
static QDomDocument createDefaultFeedList()
{
    QDomDocument doc;
    QDomProcessingInstruction z = doc.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\""));
    doc.appendChild(z);

    QDomElement root = doc.createElement(QStringLiteral("opml"));
    root.setAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    doc.appendChild(root);

    QDomElement head = doc.createElement(QStringLiteral("head"));
    root.appendChild(head);

    QDomElement text = doc.createElement(QStringLiteral("text"));
    text.appendChild(doc.createTextNode(i18n("Feeds")));
    head.appendChild(text);

    QDomElement body = doc.createElement(QStringLiteral("body"));
    root.appendChild(body);

    QDomElement mainFolder = doc.createElement(QStringLiteral("outline"));
    mainFolder.setAttribute(QStringLiteral("text"), QStringLiteral("KDE"));
    body.appendChild(mainFolder);

    QDomElement dot = doc.createElement(QStringLiteral("outline"));
    dot.setAttribute(QStringLiteral("text"), i18n("KDE Dot News"));
    dot.setAttribute(QStringLiteral("xmlUrl"), QStringLiteral("https://dot.kde.org/rss.xml"));
    mainFolder.appendChild(dot);

    QDomElement linuxFeeds = doc.createElement(QStringLiteral("outline"));
    linuxFeeds.setAttribute(QStringLiteral("text"), i18n("Linux.com"));
    linuxFeeds.setAttribute(QStringLiteral("xmlUrl"), QStringLiteral("https://www.linux.com/feed/"));
    mainFolder.appendChild(linuxFeeds);

    QDomElement planetkde = doc.createElement(QStringLiteral("outline"));
    planetkde.setAttribute(QStringLiteral("text"), i18n("Planet KDE"));
    planetkde.setAttribute(QStringLiteral("xmlUrl"), QStringLiteral("https://planetkde.org/rss20.xml"));
    mainFolder.appendChild(planetkde);

    QDomElement apps = doc.createElement(QStringLiteral("outline"));
    apps.setAttribute(QStringLiteral("text"), i18n("KDE Apps"));
    apps.setAttribute(QStringLiteral("xmlUrl"), QStringLiteral("https://store.kde.org/content.rdf"));
    mainFolder.appendChild(apps);

    // spanish feed(s)
    QDomElement spanishFolder = doc.createElement(QStringLiteral("outline"));
    spanishFolder.setAttribute(QStringLiteral("text"), i18n("Spanish feeds"));
    mainFolder.appendChild(spanishFolder);

    QDomElement spanishKde = doc.createElement(QStringLiteral("outline"));
    spanishKde.setAttribute(QStringLiteral("text"), i18n("Planet KDE España"));
    spanishKde.setAttribute(QStringLiteral("xmlUrl"), QStringLiteral("https://planet.kde-espana.org/atom.xml"));
    spanishFolder.appendChild(spanishKde);

    return doc;
}
}

namespace Akregator {
K_PLUGIN_FACTORY(AkregatorFactory, registerPlugin<Part>();
                 )

static Part *mySelf = nullptr;
BrowserExtension::BrowserExtension(Part *p, const char *name)
    : KParts::BrowserExtension(p)
{
    AkregratorMigrateApplication migrate;
    migrate.migrate();
    setObjectName(QLatin1String(name));
    m_part = p;
}

void BrowserExtension::saveSettings()
{
    m_part->saveSettings();
}

Part::Part(QWidget *parentWidget, QObject *parent, const QVariantList &)
    : KParts::ReadOnlyPart(parent)
    , m_standardListLoaded(false)
    , m_shuttingDown(false)
    , m_doCrashSave(false)
    , m_backedUpList(false)
    , m_mainWidget(nullptr)
    , m_storage(nullptr)
    , m_dialog(nullptr)
{
    mySelf = this;
    //Make sure to initialize settings
    Part::config();
    initFonts();

    setPluginLoadingMode(LoadPluginsIfEnabled);
    setPluginInterfaceVersion(AKREGATOR_PLUGIN_INTERFACE_VERSION);

    setComponentName(QStringLiteral("akregator"), i18n("Akregator"));
    setXMLFile(QStringLiteral("akregator_part.rc"), true);

    new PartAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Akregator"), this);

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/akregator/data/");
    QDir().mkpath(path);
    m_standardFeedList = path + QStringLiteral("/feeds.opml");

    Backend::StorageFactoryDummyImpl *dummyFactory = new Backend::StorageFactoryDummyImpl();
    if (!Backend::StorageFactoryRegistry::self()->registerFactory(dummyFactory, dummyFactory->key())) {
        // There was already a dummy factory registered.
        delete dummyFactory;
    }
    loadPlugins(QStringLiteral("storage"));   // FIXME: also unload them!

    m_storage = nullptr;
    Backend::StorageFactory *storageFactory = Backend::StorageFactoryRegistry::self()->getFactory(Settings::archiveBackend());
    if (storageFactory != nullptr) {
        m_storage = storageFactory->createStorage(QStringList());
    }

    if (!m_storage) { // Houston, we have a problem
        m_storage = Backend::StorageFactoryRegistry::self()->getFactory(QStringLiteral("dummy"))->createStorage(QStringList());

        KMessageBox::error(parentWidget, i18n("Unable to load storage backend plugin \"%1\". No feeds are archived.", Settings::archiveBackend()), i18n("Plugin error"));
    }

    m_storage->open(true);
    Kernel::self()->setStorage(m_storage);

    m_actionManager = new ActionManagerImpl(this);
    ActionManager::setInstance(m_actionManager);

    mCentralWidget = new Akregator::AkregatorCentralWidget(parentWidget);
    connect(mCentralWidget, &AkregatorCentralWidget::restoreSession, this, &Part::slotRestoreSession);
    m_mainWidget = new Akregator::MainWidget(this, parentWidget, m_actionManager, QStringLiteral("akregator_view"));
    mCentralWidget->setMainWidget(m_mainWidget);
    m_extension = new BrowserExtension(this, "ak_extension");

    connect(Kernel::self()->frameManager(), &FrameManager::signalCaptionChanged, this, &Part::setWindowCaption);
    connect(Kernel::self()->frameManager(), &FrameManager::signalStatusText, this, &Part::slotSetStatusText);
    connect(Kernel::self()->frameManager(), &FrameManager::signalLoadingProgress, m_extension, &BrowserExtension::loadingProgress);
    connect(Kernel::self()->frameManager(), &FrameManager::signalCanceled, this, &ReadOnlyPart::canceled);
    connect(Kernel::self()->frameManager(), &FrameManager::signalStarted, this, &Part::slotStarted);
    connect(Kernel::self()->frameManager(), SIGNAL(signalCompleted()), this, SIGNAL(completed()));

    // notify the part that this is our internal widget
    setWidget(mCentralWidget);

    //Initialize instance.
    (void)UnityServiceManager::instance();
    if (Settings::showUnreadInTaskbar()) {
        connect(m_mainWidget.data(), &MainWidget::signalUnreadCountChanged, UnityServiceManager::instance(), &UnityServiceManager::slotSetUnread);
    }

    if (Settings::showTrayIcon() && !TrayIcon::getInstance()) {
        initializeTrayIcon();
        QWidget *const notificationParent = isTrayIconEnabled() ? m_mainWidget->window() : nullptr;
        NotificationManager::self()->setWidget(notificationParent, componentData().componentName());
    }

    connect(qApp, &QCoreApplication::aboutToQuit, this, &Part::slotOnShutdown);

    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, &QTimer::timeout, this, &Part::slotSaveFeedList);
    m_autosaveTimer->start(5 * 60 * 1000); // 5 minutes

    loadPlugins(QStringLiteral("extension"));   // FIXME: also unload them!
    if (mCentralWidget->previousSessionCrashed()) {
        mCentralWidget->needToRestoreCrashedSession();
    } else {
        m_doCrashSave = true;
        autoReadProperties();
    }
}

KSharedConfig::Ptr Part::config()
{
    assert(mySelf);
    if (!mySelf->mConfig) {
        mySelf->mConfig = KSharedConfig::openConfig(QStringLiteral("akregatorrc"));
    }
    return mySelf->mConfig;
}

void Part::updateQuickSearchLineText()
{
    if (m_mainWidget) {
        m_mainWidget->updateQuickSearchLineText();
    }
}

void Part::loadPlugins(const QString &type)
{
    const KService::List offers = PluginManager::query(QStringLiteral("[X-KDE-akregator-plugintype] == '%1'").arg(type));

    for (const KService::Ptr &i : offers) {
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
    Q_EMIT started(nullptr);
}

void Part::slotOnShutdown()
{
    autoSaveProperties();
    m_shuttingDown = true;
    m_autosaveTimer->stop();
    if (m_mainWidget) {
        saveSettings();
        m_mainWidget->slotOnShutdown();
    }
    //delete m_mainWidget;
    delete TrayIcon::getInstance();
    TrayIcon::setInstance(nullptr);
    delete m_storage;
    m_storage = nullptr;
    //delete m_actionManager;
}

void Part::initializeTrayIcon()
{
    TrayIcon *trayIcon = new TrayIcon(m_mainWidget->window());
    TrayIcon::setInstance(trayIcon);
    m_actionManager->setTrayIcon(trayIcon);

    if (isTrayIconEnabled()) {
        trayIcon->setStatus(KStatusNotifierItem::Active);
    }

    connect(m_mainWidget.data(), &MainWidget::signalUnreadCountChanged, trayIcon, &TrayIcon::slotSetUnread);
    connect(m_mainWidget.data(), &MainWidget::signalArticlesSelected,
            this, &Part::signalArticlesSelected);

    m_mainWidget->slotSetTotalUnread();
}

void Part::slotSettingsChanged()
{
    if (Settings::showUnreadInTaskbar()) {
        connect(m_mainWidget.data(), &MainWidget::signalUnreadCountChanged, UnityServiceManager::instance(), &UnityServiceManager::slotSetUnread);
        m_mainWidget->slotSetTotalUnread();
    } else {
        disconnect(m_mainWidget.data(), &MainWidget::signalUnreadCountChanged, UnityServiceManager::instance(), &UnityServiceManager::slotSetUnread);
        UnityServiceManager::instance()->slotSetUnread(0);
    }

    NotificationManager::self()->setWidget(isTrayIconEnabled() ? m_mainWidget->window() : nullptr, componentData().componentName());

    if (Settings::showTrayIcon()) {
        if (!TrayIcon::getInstance()) {
            initializeTrayIcon();
            m_mainWidget->slotSetTotalUnread();
        }
    } else {
        TrayIcon::getInstance()->disconnect();
        delete TrayIcon::getInstance();
        TrayIcon::setInstance(nullptr);
        m_actionManager->setTrayIcon(nullptr);
    }

    const QStringList fonts {
        Settings::standardFont(),
        Settings::fixedFont(),
        Settings::sansSerifFont(),
        Settings::serifFont(),
        Settings::standardFont(),
        Settings::standardFont(),
        QStringLiteral("0")
    };
    Settings::setFonts(fonts);

    if (Settings::minimumFontSize() > Settings::mediumFontSize()) {
        Settings::setMediumFontSize(Settings::minimumFontSize());
    }
    saveSettings();
    Q_EMIT signalSettingsChanged();

    initFonts();
}

void Part::slotSetStatusText(const QString &statusText)
{
    KPIM::BroadcastStatus::instance()->setStatusMsg(statusText);
}

void Part::saveSettings()
{
    if (m_mainWidget) {
        m_mainWidget->saveSettings();
    }
}

Part::~Part()
{
    disconnect(qApp, &QCoreApplication::aboutToQuit, this, &Part::slotOnShutdown);
    qCDebug(AKREGATOR_LOG) << "Part::~Part() enter";
    // If the widget is destroyed for some reason, KParts::Part will set its
    // widget property to 0 and then delete itself (and therefore this object).
    // In this case, it's not safe to do our normal shutdown routine.
    if (widget() && !m_shuttingDown) {
        slotOnShutdown();
    }
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

void Part::exportFile(const QString &str)
{
    exportFile(QUrl(str));
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
    connect(cmd.data(), &LoadFeedListCommand::result,
            this, &Part::feedListLoaded);
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
    if (!m_mainWidget) {
        return;
    }

    m_mainWidget->setFeedList(list);
    m_standardListLoaded = list != nullptr;

    if (Settings::markAllFeedsReadOnStartup()) {
        m_mainWidget->slotMarkAllFeedsRead();
    }

    if (m_standardListLoaded) {
        QTimer::singleShot(0, this, &Part::flushAddFeedRequests);
    }

    if (Settings::fetchOnStartup()) {
        m_mainWidget->slotFetchAllFeeds();
    }
}

void Part::flushAddFeedRequests()
{
    if (!m_mainWidget) {
        return;
    }

    for (const AddFeedRequest &i : qAsConst(m_requests)) {
        for (const QString &j : qAsConst(i.urls)) {
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
        const QString backup = localFilePath() + QLatin1Char('~');
        if (QFile::exists(backup)) {
            QFile::remove(backup);
        }
        if (QFile::copy(localFilePath(), backup)) {
            m_backedUpList = true;
        }
    }

    const QString xml = m_mainWidget->feedListToOPML().toString();
    if (xml.isEmpty()) {
        return;
    }

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
    QTemporaryFile tempFile;

    if (url.isLocalFile()) {
        filename = url.toLocalFile();
    } else {
        if (!tempFile.open()) {
            return;
        }
        filename = tempFile.fileName();

        auto job = KIO::file_copy(url, QUrl::fromLocalFile(filename), -1, KIO::Overwrite | KIO::HideProgressInfo);
        KJobWidgets::setWindow(job, m_mainWidget);
        if (!job->exec()) {
            KMessageBox::error(m_mainWidget, job->errorString());
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
}

void Part::exportFile(const QUrl &url)
{
    if (url.isLocalFile()) {
        const QString fname = url.toLocalFile();

        if (!writeToTextFile(m_mainWidget->feedListToOPML().toString(), fname)) {
            KMessageBox::error(m_mainWidget, i18n("Access denied: cannot write to file %1. Please check your permissions.", fname), i18n("Write Error"));
        }

        return;
    } else {
        auto job = KIO::storedPut(m_mainWidget->feedListToOPML().toString().toUtf8(), url, -1);
        KJobWidgets::setWindow(job, m_mainWidget);
        if (!job->exec()) {
            KMessageBox::error(m_mainWidget, job->errorString());
        }
    }
}

void Part::fileImport()
{
    const QString filters = i18n("OPML Outlines (*.opml *.xml);;All Files (*)");
    const QUrl url = QFileDialog::getOpenFileUrl(m_mainWidget, QString(), QUrl(), filters);
    if (!url.isEmpty()) {
        importFile(url);
    }
}

void Part::fileExport()
{
    const QString filters = i18n("OPML Outlines (*.opml *.xml);;All Files (*)");
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
        connect(m_dialog, qOverload<>(&KCMultiDialog::configCommitted),
                this, &Part::slotSettingsChanged);
        if (TrayIcon::getInstance()) {
            connect(m_dialog, qOverload<>(&KCMultiDialog::configCommitted),
                    TrayIcon::getInstance(), &TrayIcon::settingsChanged);
        }

        m_dialog->addModule(QStringLiteral("akregator_config_general"));
        m_dialog->addModule(QStringLiteral("akregator_config_appearance"));
        m_dialog->addModule(QStringLiteral("akregator_config_archive"));
        m_dialog->addModule(QStringLiteral("akregator_config_browser"));
        m_dialog->addModule(QStringLiteral("akregator_config_advanced"));
        m_dialog->addModule(QStringLiteral("akregator_config_plugins"));
    }

    m_dialog->show();
    m_dialog->raise();
}

void Part::initFonts()
{
    QStringList fonts = Settings::fonts();
    if (fonts.isEmpty()) {
        fonts.append(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family());
        fonts.append(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
        fonts.append(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family());
        fonts.append(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family());
        fonts.append(QStringLiteral("0"));
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

    //TODO add CursiveFont, FantasyFont

    KConfigGroup conf(Settings::self()->config(), "HTML Settings");

    KConfig _konq(QStringLiteral("konquerorrc"), KConfig::NoGlobals);
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
    QWebEngineSettings::defaultSettings()->setFontFamily(QWebEngineSettings::StandardFont, Settings::standardFont());
    QWebEngineSettings::defaultSettings()->setFontFamily(QWebEngineSettings::FixedFont, Settings::fixedFont());
    QWebEngineSettings::defaultSettings()->setFontFamily(QWebEngineSettings::SerifFont, Settings::serifFont());
    QWebEngineSettings::defaultSettings()->setFontFamily(QWebEngineSettings::SansSerifFont, Settings::sansSerifFont());
    QWebEngineSettings::defaultSettings()->setFontSize(QWebEngineSettings::MinimumFontSize, Settings::minimumFontSize());
    QWebEngineSettings::defaultSettings()->setFontSize(QWebEngineSettings::DefaultFontSize, Settings::mediumFontSize());
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
        const auto positionalArguments = parser.positionalArguments();
        for (QString url : positionalArguments) {
            const QUrl tempUrl = QUrl::fromUserInput(url);
            if (tempUrl.isLocalFile()) {
                const QString tempLocalFile = tempUrl.toLocalFile();
                if (tempLocalFile.startsWith(QDir::tempPath())) {
                    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/akregator/data/");
                    QDir().mkpath(path);
                    QFile f(tempLocalFile);
                    const QString newRssFileName = path + QFileInfo(f).fileName();
                    if (!f.copy(newRssFileName)) {
                        qCWarning(AKREGATOR_LOG) << "Impossible to copy in local folder" << newRssFileName;
                    } else {
                        url = newRssFileName;
                    }
                }
            }

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
    KConfig config(QStringLiteral("crashed"), KConfig::SimpleConfig,
                   QStandardPaths::AppDataLocation);
    KConfigGroup configGroup(&config, "Part");
    configGroup.writeEntry("crashed", false);
}

void Part::saveCrashProperties()
{
    if (!m_doCrashSave) {
        return;
    }
    KConfig config(QStringLiteral("crashed"), KConfig::SimpleConfig,
                   QStandardPaths::AppDataLocation);
    KConfigGroup configGroup(&config, "Part");
    configGroup.deleteGroup();

    configGroup.writeEntry("crashed", true);

    saveProperties(configGroup);
}

void Part::slotAutoSave()
{
    saveCrashProperties();
}

void Part::autoSaveProperties()
{
    KConfig config(QStringLiteral("autosaved"), KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
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
    KConfig config(QStringLiteral("autosaved"), KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
    KConfigGroup configGroup(&config, "Part");

    readProperties(configGroup);
}

void Part::slotRestoreSession(Akregator::CrashWidget::CrashAction type)
{
    switch (type) {
    case Akregator::CrashWidget::RestoreSession:
    {
        KConfig config(QStringLiteral("crashed"), KConfig::SimpleConfig,
                       QStandardPaths::AppDataLocation);
        KConfigGroup configGroup(&config, "Part");
        readProperties(configGroup);
        clearCrashProperties();
        break;
    }
    case Akregator::CrashWidget::NotRestoreSession:
        clearCrashProperties();
        break;
    case Akregator::CrashWidget::AskMeLater:
        break;
    }
    m_doCrashSave = true;
}
} // namespace Akregator
#include "akregator_part.moc"
