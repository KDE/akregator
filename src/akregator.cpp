/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "app.h"
#include "akregator.h"
#include "trayicon.h"
#include "akregatorconfig.h"

#include <kkeydialog.h>
#include <kfiledialog.h>
#include <kprogress.h>
#include <kconfig.h>
#include <kurl.h>

#include <kedittoolbar.h>

#include <kaction.h>
#include <kstdaction.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kdebug.h>

using namespace Akregator;

BrowserInterface::BrowserInterface( aKregator *shell, const char *name )
    : KParts::BrowserInterface( shell, name )
{
    m_shell = shell;
}

bool BrowserInterface::haveWindowLoaded() const
{
    return akreapp->haveWindowLoaded();
}

aKregator::aKregator()
    : KParts::MainWindow( 0L, "aKregator" )
    , m_quit(false)
{
    // set the shell's ui resource file
    setXMLFile("akregator_shell.rc");

    m_browserIface=new BrowserInterface(this, "browser_interface");

    // then, setup our actions
    setupActions();

    // and a status bar
    statusBar()->show();

    m_progressBar = new KProgress( this );
    m_progressBar->setMaximumHeight(fontMetrics().height());
    m_progressBar->hide();
    statusBar()->addWidget( m_progressBar, 0, true);

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory("libakregatorpart");
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<KParts::ReadWritePart*>(factory->create(this, "akregator_part", "KParts::ReadWritePart" ));

        if (m_part)
        {
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget(m_part->widget());

            connect (m_part, SIGNAL(partChanged(KParts::Part *)), this, SLOT(partChanged(KParts::Part *)));
            connect( browserExtension(), SIGNAL(loadingProgress(int)), this, SLOT(loadingProgress(int)) );

            // and integrate the part's GUI with the shell's
            createGUI(m_part);
            browserExtension()->setBrowserInterface(m_browserIface);
        }
    }
    else
    {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Could not find our part."));
        kapp->quit();
        // we return here, cause kapp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

    TrayIcon *icon = new TrayIcon(this);
    icon->show();
    connect(icon, SIGNAL(quitSelected()),
            this, SLOT(quitProgram()));

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();

    akreapp->setHaveWindowLoaded(true);
}

void aKregator::loadLastOpenFile()
{
   load( Settings::lastOpenFile() );
}

aKregator::~aKregator()
{
   Settings::setLastOpenFile( m_part->url().url() );
   Settings::writeConfig();
}

void aKregator::partChanged(KParts::Part *p)
{
    createGUI(p);
}

void aKregator::load(const KURL& url)
{
    m_part->openURL( url );
}

void aKregator::setupActions()
{
    KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
//    KStdAction::open(this, SLOT(fileOpen()), actionCollection());

    KStdAction::quit(this, SLOT(quitProgram()), actionCollection());

    m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
    m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

    // TODO: move to config dialog when it arrives
    m_fetchStartupAction = new KToggleAction(i18n("&Fetch Feeds on Startup"), "", "", this, SLOT(optionsFetchOnStartup()), actionCollection(), "fetch_on_startup");

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());

    m_fetchStartupAction->setChecked(Settings::fetchOnStartup());
}

void aKregator::saveProperties(KConfig* /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void aKregator::readProperties(KConfig* /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

void aKregator::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // About this function, the style guide (
    // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
    // says that it should open a new window if the document is _not_
    // in its initial state.  This is what we do here..
    if ( ! m_part->url().isEmpty() || m_part->isModified() )
    {
        m_part->save();
        (new aKregator)->show();
    };
}


void aKregator::optionsShowToolbar()
{
    // this is all very cut and paste code for showing/hiding the
    // toolbar
    if (m_toolbarAction->isChecked())
        toolBar()->show();
    else
        toolBar()->hide();
}

void aKregator::optionsShowStatusbar()
{
    // this is all very cut and paste code for showing/hiding the
    // statusbar
    if (m_statusbarAction->isChecked())
        statusBar()->show();
    else
        statusBar()->hide();
}

void aKregator::optionsConfigureKeys()
{
    KKeyDialog dlg( true, this );

    dlg.insert(actionCollection());
    if (m_part)
        dlg.insert(m_part->actionCollection());

    dlg.configure();
}

void aKregator::optionsConfigureToolbars()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    saveMainWindowSettings(KGlobal::config() );
# endif
#else
    saveMainWindowSettings(KGlobal::config() );
#endif

    // use the standard toolbar editor
    KEditToolbar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

void aKregator::applyNewToolbarConfig()
{
#if defined(KDE_MAKE_VERSION)
# if KDE_VERSION >= KDE_MAKE_VERSION(3,1,0)
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
# else
    applyMainWindowSettings(KGlobal::config());
# endif
#else
    applyMainWindowSettings(KGlobal::config());
#endif
}

void aKregator::optionsFetchOnStartup()
{
   Settings::setFetchOnStartup(m_fetchStartupAction->isChecked());
   Settings::writeConfig();
}

void aKregator::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    KURL url =
        KFileDialog::getOpenURL( QString::null, QString::null, this );

    if (url.isEmpty() == false)
    {
        // About this function, the style guide (
        // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
        // says that it should open a new window if the document is _not_
        // in its initial state.  This is what we do here..
        if ( m_part->url().isEmpty() && ! m_part->isModified() )
        {
            // we open the file in this window...
            load( url );
        }
        else
        {
            // we open the file in a new window...
            aKregator* newWin = new aKregator;
            newWin->load( url );
            newWin->show();
        }
    }
}

KParts::BrowserExtension *aKregator::browserExtension() const
{
    return KParts::BrowserExtension::childObject( m_part );
}


void aKregator::quitProgram()
{
    // will call queryClose()
    m_quit = true;
    close();
}

// from KonqFrameStatusBar
void aKregator::fontChange(const QFont & /* oldFont */)
{
    int h = fontMetrics().height();
    if ( h < 13 ) h = 13;
    m_progressBar->setFixedHeight( h + 2 );

}

void aKregator::loadingProgress(int percent)
{
    if ( percent != -1 && percent != 100 ) // hide on 100 too
    {
        if ( !m_progressBar->isVisible() )
            m_progressBar->show();
    }
    else
        m_progressBar->hide();

    m_progressBar->setValue( percent );
}

void aKregator::closeEvent(QCloseEvent* e)
{
    if (!m_quit)
    {
        KMessageBox::information(this, i18n( "<qt>Closing the main window will keep aKregator running in the system tray. Use 'Quit' from the 'File' menu to quit the application.</qt>" ), i18n( "Docking in System Tray" ), "hideOnCloseInfo");
        hide();
        e->ignore();
    }
    else
    {
        if (m_part->queryClose())
            KMainWindow::closeEvent(e);
    }
    m_quit = false;
}

#include "akregator.moc"
