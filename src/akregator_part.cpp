/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akregator_part.h"
#include "akregator_debug.h"

#include <MessageViewer/MessageViewerSettings>

#include "aboutdata.h"
#include "actionmanagerimpl.h"
#include "akregator-version.h"
#include "akregator_options.h"
#include "akregatorconfig.h"
#include "article.h"
#include "feedlist.h"
#include "fetchqueue.h"
#include "framemanager.h"
#include "kernel.h"
#include "loadfeedlistcommand.h"
#include "mainwidget.h"
#include "notificationmanager.h"
#include "storage/storage.h"
#include "trayicon.h"
#include "unityservicemanager.h"
#include "utils.h"
#include "widgets/akregatorcentralwidget.h"
#include <KConfig>
#include <KIO/FileCopyJob>
#include <KMessageBox>
#include <KNotifyConfigWidget>
#include <KPluginMetaData>
#include <PimCommon/BroadcastStatus>
#include <QFileDialog>

#include <KCMultiDialog>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KParts/Plugin>
#include <KPluginFactory>
#include <KService>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QWebEngineSettings>
#include <kxmlguifactory.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "akregratormigrateapplication.h"
#endif
#include "partadaptor.h"
#include <QApplication>
#include <QDomDocument>
#include <QFile>
#include <QObject>
#include <QStringList>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QWebEngineProfile>
#endif

#include <QFontDatabase>
#include <QStandardPaths>
#include <memory>

namespace
{
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
    planetkde.setAttribute(QStringLiteral("xmlUrl"), QStringLiteral("https://planet.kde.org/global/atom.xml"));
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

namespace Akregator
{
K_PLUGIN_FACTORY(AkregatorFactory, registerPlugin<Part>();)

static Part *mySelf = nullptr;

Part::Part(QWidget *parentWidget, QObject *parent, const QVariantList &)
    : KParts::Part(parent)
{
    mySelf = this;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    AkregratorMigrateApplication migrate;
    migrate.migrate();
#endif
    // Make sure to initialize settings
    Part::config();
    initFonts();

    setComponentName(QStringLiteral("akregator"), i18n("Akregator"));
    setXMLFile(QStringLiteral("akregator_part.rc"), true);

    new PartAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Akregator"), this);

    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/akregator/data/");
    QDir().mkpath(path);
    m_standardFeedList = path + QStringLiteral("/feeds.opml");

    m_storage = new Backend::Storage;
    m_storage->open(true);

    Kernel::self()->setStorage(m_storage);

    m_actionManager = new ActionManagerImpl(this);
    ActionManager::setInstance(m_actionManager);

    mCentralWidget = new Akregator::AkregatorCentralWidget(parentWidget);
    connect(mCentralWidget, &AkregatorCentralWidget::restoreSession, this, &Part::slotRestoreSession);
    m_mainWidget = new Akregator::MainWidget(this, parentWidget, m_actionManager, QStringLiteral("akregator_view"));
    mCentralWidget->setMainWidget(m_mainWidget);

    connect(Kernel::self()->frameManager(), &FrameManager::signalCaptionChanged, this, &Part::setWindowCaption);
    connect(Kernel::self()->frameManager(), &FrameManager::signalStatusText, this, &Part::slotSetStatusText);

    // notify the part that this is our internal widget
    setWidget(mCentralWidget);

    // Initialize instance.
    (void)UnityServiceManager::instance();
    if (Settings::showUnreadInTaskbar()) {
        connect(m_mainWidget.data(), &MainWidget::signalUnreadCountChanged, UnityServiceManager::instance(), &UnityServiceManager::slotSetUnread);
    }

    if (Settings::showTrayIcon() && !TrayIcon::getInstance()) {
        initializeTrayIcon();
        QWidget *const notificationParent = isTrayIconEnabled() ? m_mainWidget->window() : nullptr;
        NotificationManager::self()->setWidget(notificationParent, componentName());
    }

    connect(qApp, &QCoreApplication::aboutToQuit, this, &Part::slotOnShutdown);

    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, &QTimer::timeout, this, &Part::slotSaveFeedList);
    m_autosaveTimer->start(5 * 60 * 1000); // 5 minutes

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

void Part::slotOnShutdown()
{
    autoSaveProperties();
    m_shuttingDown = true;
    m_autosaveTimer->stop();
    if (m_mainWidget) {
        saveSettings();
        m_mainWidget->slotOnShutdown();
    }
    // delete m_mainWidget;
    delete TrayIcon::getInstance();
    TrayIcon::setInstance(nullptr);
    delete m_storage;
    m_storage = nullptr;
    // delete m_actionManager;
}

void Part::initializeTrayIcon()
{
    auto trayIcon = new TrayIcon(m_mainWidget->window());
    TrayIcon::setInstance(trayIcon);
    m_actionManager->setTrayIcon(trayIcon);

    trayIcon->setEnabled(isTrayIconEnabled());

    connect(m_mainWidget.data(), &MainWidget::signalUnreadCountChanged, trayIcon, &TrayIcon::slotSetUnread);
    connect(m_mainWidget.data(), &MainWidget::signalArticlesSelected, this, &Part::signalArticlesSelected);

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

    NotificationManager::self()->setWidget(isTrayIconEnabled() ? m_mainWidget->window() : nullptr, componentName());
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

    const QStringList fonts{Settings::standardFont(),
                            Settings::fixedFont(),
                            Settings::sansSerifFont(),
                            Settings::serifFont(),
                            Settings::standardFont(),
                            Settings::standardFont(),
                            QStringLiteral("0")};
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
    PimCommon::BroadcastStatus::instance()->setStatusMsg(statusText);
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

void Part::openStandardFeedList()
{
    if (!m_standardFeedList.isEmpty()) {
        openFile(m_standardFeedList);
    }
}

void Part::openFile(const QString &filePath)
{
    if (m_loadFeedListCommand || m_standardListLoaded) {
        return;
    }
    QScopedPointer<LoadFeedListCommand> cmd(new LoadFeedListCommand(m_mainWidget));
    cmd->setParentWidget(m_mainWidget);
    cmd->setStorage(Kernel::self()->storage());
    cmd->setFileName(filePath);
    cmd->setDefaultFeedList(createDefaultFeedList());
    connect(cmd.data(), &LoadFeedListCommand::result, this, &Part::feedListLoaded);
    m_loadFeedListCommand = cmd.take();
    m_loadFeedListCommand->start();
}

bool Part::writeToTextFile(const QString &data, const QString &filename) const
{
    QSaveFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    stream << data << Qt::endl;
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

    for (const AddFeedRequest &i : std::as_const(m_requests)) {
        for (const QString &j : std::as_const(i.urls)) {
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
        const QString backup = m_standardFeedList + QLatin1Char('~');
        if (QFile::exists(backup)) {
            QFile::remove(backup);
        }
        if (QFile::copy(m_standardFeedList, backup)) {
            m_backedUpList = true;
        }
    }

    const QString xml = m_mainWidget->feedListToOPML().toString();
    if (xml.isEmpty()) {
        return;
    }

    m_storage->storeFeedList(xml);
    if (writeToTextFile(xml, m_standardFeedList)) {
        return;
    }

    KMessageBox::error(m_mainWidget,
                       i18n("Access denied: Cannot save feed list to <b>%1</b>. Please check your permissions.", m_standardFeedList),
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
        KMessageBox::error(m_mainWidget,
                           i18n("The file %1 could not be read, check if it exists or if it is readable for the current user.", filename),
                           i18n("Read Error"));
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
    const QString filters = i18n("OPML Outlines (%1);;All Files (*)", QStringLiteral("*.opml *.xml"));
    const QUrl url = QFileDialog::getOpenFileUrl(m_mainWidget, QString(), QUrl(), filters);
    if (!url.isEmpty()) {
        importFile(url);
    }
}

void Part::fileExport()
{
    const QString filters = i18n("OPML Outlines (%1);;All Files (*)", QStringLiteral("*.opml *.xml"));
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
        connect(m_dialog, qOverload<>(&KCMultiDialog::configCommitted), this, &Part::slotSettingsChanged);
        if (TrayIcon::getInstance()) {
            connect(m_dialog, qOverload<>(&KCMultiDialog::configCommitted), TrayIcon::getInstance(), &TrayIcon::settingsChanged);
        }
        const QVector<KPluginMetaData> availablePlugins = KPluginMetaData::findPlugins(QStringLiteral("pim/kcms/akregator"));
        for (const KPluginMetaData &metaData : availablePlugins) {
            m_dialog->addModule(metaData);
        }
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

    // TODO add CursiveFont, FantasyFont

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QWebEngineSettings::defaultSettings()->setFontFamily(QWebEngineSettings::StandardFont, Settings::standardFont());
    QWebEngineSettings::defaultSettings()->setFontFamily(QWebEngineSettings::FixedFont, Settings::fixedFont());
    QWebEngineSettings::defaultSettings()->setFontFamily(QWebEngineSettings::SerifFont, Settings::serifFont());
    QWebEngineSettings::defaultSettings()->setFontFamily(QWebEngineSettings::SansSerifFont, Settings::sansSerifFont());
    QWebEngineSettings::defaultSettings()->setFontSize(QWebEngineSettings::MinimumFontSize, Settings::minimumFontSize());
    QWebEngineSettings::defaultSettings()->setFontSize(QWebEngineSettings::DefaultFontSize, Settings::mediumFontSize());
#else
    QWebEngineProfile::defaultProfile()->settings()->setFontFamily(QWebEngineSettings::StandardFont, Settings::standardFont());
    QWebEngineProfile::defaultProfile()->settings()->setFontFamily(QWebEngineSettings::FixedFont, Settings::fixedFont());
    QWebEngineProfile::defaultProfile()->settings()->setFontFamily(QWebEngineSettings::SerifFont, Settings::serifFont());
    QWebEngineProfile::defaultProfile()->settings()->setFontFamily(QWebEngineSettings::SansSerifFont, Settings::sansSerifFont());
    QWebEngineProfile::defaultProfile()->settings()->setFontSize(QWebEngineSettings::MinimumFontSize, Settings::minimumFontSize());
    QWebEngineProfile::defaultProfile()->settings()->setFontSize(QWebEngineSettings::DefaultFontSize, Settings::mediumFontSize());
#endif
}

bool Part::handleCommandLine(const QStringList &args)
{
    QCommandLineParser parser;
    akregator_options(&parser);
    parser.process(args);

    const QString addFeedGroup = parser.isSet(QStringLiteral("group")) ? parser.value(QStringLiteral("group")) : i18n("Imported Folder");

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
    KConfig config(QStringLiteral("crashed"), KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
    KConfigGroup configGroup(&config, "Part");
    configGroup.writeEntry("crashed", false);
}

void Part::saveCrashProperties()
{
    if (!m_doCrashSave) {
        return;
    }
    KConfig config(QStringLiteral("crashed"), KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
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
    case Akregator::CrashWidget::RestoreSession: {
        KConfig config(QStringLiteral("crashed"), KConfig::SimpleConfig, QStandardPaths::AppDataLocation);
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
