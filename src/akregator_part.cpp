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

#include "akregatorconfig.h"
#include "akregator_part.h"
#include "aboutdata.h"
#include "actionmanagerimpl.h"
#include "feediconmanager.h"
#include "framemanager.h"
#include "kernel.h"
#include "mainwidget.h"
#include "notificationmanager.h"
#include "plugin.h"
#include "pluginmanager.h"
#include "setupakonadicommand.h"
#include "trayicon.h"

#include <krss/feedlist.h>
#include <krss/resourcemanager.h>
#include <krss/tagprovider.h>

#include <syndication/dataretriever.h>

#include <libkdepim/broadcaststatus.h>

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

#include <memory>

using namespace boost;

namespace Akregator {

static const KAboutData &createAboutData()
{
  static Akregator::AboutData about;
  return about;
}

K_PLUGIN_FACTORY(AkregatorFactory, registerPlugin<Part>();)
K_EXPORT_PLUGIN(AkregatorFactory(createAboutData()))

BrowserExtension::BrowserExtension( Part *p )
  : KParts::BrowserExtension( p )
  , m_part( p )
{
}

void BrowserExtension::saveSettings()
{
    m_part->saveSettings();
}

Part::Part( QWidget *parentWidget, QObject *parent, const QVariantList& )
    : inherited(parent)
    , m_shuttingDown(false)
    , m_extension( new BrowserExtension( this ) )
    , m_parentWidget(parentWidget)
    , m_actionManager( new ActionManagerImpl( this ) )
    , m_mainWidget()
    , m_dialog()

{
    initFonts();

    setPluginLoadingMode( LoadPluginsIfEnabled );
    setPluginInterfaceVersion( AKREGATOR_PLUGIN_INTERFACE_VERSION );

    setComponentData( AkregatorFactory::componentData() );
    setXMLFile("akregator_part.rc", true);

    new PartAdaptor( this );
    QDBusConnection::sessionBus().registerObject("/Akregator", this);

    FeedIconManager::self(); // FIXME: registering the icon manager dbus iface here,
                               // because otherwise we get a deadlock later

    ActionManager::setInstance( m_actionManager );

    m_mainWidget = new MainWidget( this, m_parentWidget, m_actionManager );

    connect(Kernel::self()->frameManager(), SIGNAL(signalCaptionChanged(const QString&)), this, SIGNAL(setWindowCaption(const QString&)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalStatusText(const QString&)), this, SLOT(slotSetStatusText(const QString&)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalLoadingProgress(int)), m_extension, SIGNAL(loadingProgress(int)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalCanceled(const QString&)), this, SIGNAL(canceled(const QString&)));
    connect(Kernel::self()->frameManager(), SIGNAL(signalStarted()), this, SLOT(slotStarted()));
    connect(Kernel::self()->frameManager(), SIGNAL(signalCompleted()), this, SIGNAL(completed()));

    // notify the part that this is our internal widget
    setWidget(m_mainWidget);

    TrayIcon* trayIcon = new TrayIcon( m_mainWidget->window() );
    TrayIcon::setInstance(trayIcon);
    m_actionManager->initTrayIcon(trayIcon);
    connect( trayIcon, SIGNAL(quitSelected()),
             kapp, SLOT(quit()) );

    connect( m_mainWidget, SIGNAL(signalUnreadCountChanged(int)),
             trayIcon, SLOT(slotSetUnread(int)) );

    if ( isTrayIconEnabled() )
        trayIcon->show();

    QWidget* const notificationParent = isTrayIconEnabled() ? m_mainWidget->window() : 0;
    NotificationManager::self()->setWidget(notificationParent, componentData());


    connect(kapp, SIGNAL(aboutToQuit()), this, SLOT(slotOnShutdown()));

    loadPlugins( QLatin1String("extension") ); // FIXME: also unload them!
}

void Part::loadPlugins( const QString& type )
{
    const KService::List offers = PluginManager::query( QString::fromLatin1("[X-KDE-akregator-plugintype] == '%1'").arg( type ) );

    Q_FOREACH ( const KService::Ptr& i, offers ) {
        Akregator::Plugin* plugin = PluginManager::createFromService( i );
        if ( !plugin )
            continue;
        plugin->initialize();
        plugin->insertGuiClients( this );
    }
}

void Part::slotAkonadiSetUp( KJob* job ) {
    if ( job->error() ) {
        QApplication::quit();
        return;
    }
    KRss::RetrieveFeedListJob * const fjob = new KRss::RetrieveFeedListJob( m_mainWidget );
    fjob->setResources( KRss::ResourceManager::self()->resources() );
    connect( fjob, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedListRetrieved( KJob* ) ) );
    fjob->start();

    KRss::TagProviderRetrieveJob * const tjob = new KRss::TagProviderRetrieveJob( m_mainWidget );
    connect( tjob, SIGNAL( result( KJob* ) ), this, SLOT( slotTagProviderRetrieved( KJob* ) ) );
    tjob->start();
}

void Part::slotStarted()
{
    emit started(0L);
}

void Part::slotOnShutdown()
{
    m_shuttingDown = true;
    saveSettings();
    if ( m_mainWidget )
        m_mainWidget->slotOnShutdown();
    //delete m_mainWidget;
    delete TrayIcon::getInstance();
    TrayIcon::setInstance(0L);
    //delete m_actionManager;
}

void Part::addFeed() {
}

void Part::slotSettingsChanged()
{
    NotificationManager::self()->setWidget(isTrayIconEnabled() ? m_mainWidget->window() : 0, componentData());

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

void Part::slotSetStatusText( const QString& statusText )
{
  KPIM::BroadcastStatus::instance()->setStatusMsg( statusText );
}

void Part::saveSettings()
{
    if ( m_mainWidget )
        m_mainWidget->saveSettings();
}

Part::~Part()
{
    if (!m_shuttingDown)
        slotOnShutdown();
    delete m_dialog;
}

void Part::readProperties(const KConfigGroup & config)
{
    openStandardFeedList();

    if(m_mainWidget)
        m_mainWidget->readProperties(config);
}

void Part::saveProperties(KConfigGroup & config)
{
    if (m_mainWidget)
        m_mainWidget->saveProperties(config);
}

bool Part::openUrl(const KUrl& url)
{
    setLocalFilePath(url.toLocalFile());
    return openFile();
}

void Part::openStandardFeedList()
{
    SetUpAkonadiCommand* cmd = new SetUpAkonadiCommand;
    cmd->setParentWidget( m_mainWidget );
    cmd->setMainWidget( m_mainWidget );
    connect( cmd, SIGNAL(finished(KJob*)), this , SLOT(slotAkonadiSetUp(KJob*)) );
    cmd->start();
}

bool Part::openFile() {
    return true;
}

bool Part::isTrayIconEnabled() const
{
    return Settings::showTrayIcon();
}

void Part::showNotificationOptions()
{
    const Akregator::AboutData about;
    KNotifyConfigWidget::configure(m_mainWidget, about.appName() );
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
            minfs = konq.readEntry("MinimumFontSize", 8);
        else
            minfs = std::max( KGlobalSettings::generalFont().pointSize() - 2, 4 );
        Settings::setMinimumFontSize(minfs);
    }

    if (!conf.hasKey("MediumFontSize"))
    {
        int medfs;
        if (konq.hasKey("MediumFontSize"))
            medfs = konq.readEntry("MediumFontSize", 12);
        else
            medfs = KGlobalSettings::generalFont().pointSize();
        Settings::setMediumFontSize(medfs);
    }

    if (!conf.hasKey("UnderlineLinks"))
    {
        bool underline = true;
        if (konq.hasKey("UnderlineLinks"))
            underline = konq.readEntry("UnderlineLinks", false);
        Settings::setUnderlineLinks(underline);
    }

}

void Part::slotTagProviderRetrieved( KJob *job )
{
    const KRss::TagProviderRetrieveJob* const rjob = qobject_cast<const KRss::TagProviderRetrieveJob*>( job );
    assert( rjob );

    if ( rjob->error() ) {
        KMessageBox::error( m_mainWidget, i18n( "Could not retrieve the tag provider. %1",
                                                rjob->errorString() ) );
        return;
    }

    m_mainWidget->setTagProvider( rjob->tagProvider() );
}

void Part::slotFeedListRetrieved( KJob *job )
{
    const KRss::RetrieveFeedListJob * const rjob = qobject_cast<const KRss::RetrieveFeedListJob*>( job );
    assert( rjob );

    if ( rjob->error() ) {
        KMessageBox::error( m_mainWidget, i18n( "Could not retrieve the feed list. %1", rjob->errorString() ) );
        return;
    }

    m_mainWidget->setFeedList( rjob->feedList() );

    if( Settings::markAllFeedsReadOnStartup() )
        m_mainWidget->slotMarkAllFeedsRead();

    if (Settings::fetchOnStartup())
        m_mainWidget->slotFetchAllFeeds();
}

} // namespace Akregator

#include "akregator_part.moc"
