/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#include "mainwindow.h"
#include "akregator_part.h"
#include "akregatorconfig.h"
#include "trayicon.h"

#include "libkdepim/misc/broadcaststatus.h"
#include "libkdepim/progresswidget/progressstatusbarwidget.h"
#include "libkdepim/progresswidget/statusbarprogresswidget.h"

#include <KAction>
#include <KActionCollection>
#include <KApplication>
#include <KConfig>
#include <KEditToolBar>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSqueezedTextLabel>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KStatusBar>
#include <KPluginLoader>
#include <KMenuBar>
#include <KXMLGUIFactory>
#include <KToolBar>
#include <KPluginFactory>

using namespace Akregator;

BrowserInterface::BrowserInterface( MainWindow *shell, const char *name )
    : KParts::BrowserInterface( shell )
{
    setObjectName(QLatin1String(name));
    m_shell = shell;
}

MainWindow::MainWindow( QWidget* parent, Qt::WindowFlags f )
    : KParts::MainWindow( parent, f )
    , m_browserIface( new BrowserInterface( this, "browser_interface" ) )
    , m_part()
    , m_statusLabel( new KSqueezedTextLabel( this ) )
{
    setPluginLoadingMode( DoNotLoadPlugins );

    // set the shell's ui resource file
    setXMLFile(QLatin1String("akregator_shell.rc"));

    KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection()); // options_configure_keybinding
    KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection()); // options_configure_toolbars

    toolBar()->show();
    statusBar()->show();

    int statH=fontMetrics().height()+2;
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setSizePolicy(QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed ));
    m_statusLabel->setMinimumWidth( 0 );
    m_statusLabel->setFixedHeight( statH );
    statusBar()->addWidget( m_statusLabel, 1 );

    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    KStandardAction::showMenubar( menuBar(), SLOT(setVisible(bool)), actionCollection());
    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();

    connect( KPIM::BroadcastStatus::instance(), SIGNAL(statusMsg(QString)),
             this, SLOT(slotSetStatusBarText(QString)) );
}

bool MainWindow::loadPart()
{
    if ( m_part )
        return true;
    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KPluginLoader loader(QLatin1String("akregatorpart"));
    KPluginFactory* const factory = loader.factory();
    if (!factory) {
        KMessageBox::error(this, i18n("Could not find the Akregator part; please check your installation.\n%1", loader.errorString()) );
        return false;
    }

    m_part = static_cast<Akregator::Part*>( factory->create<KParts::ReadOnlyPart>(this) );

    if ( !m_part )
        return false;

    m_part->setObjectName(QLatin1String("akregator_part"));
    setCentralWidget(m_part->widget());

    connect(m_part, SIGNAL(setWindowCaption(QString)), this, SLOT(setCaption(QString)) );

    createGUI(m_part);
    browserExtension(m_part)->setBrowserInterface(m_browserIface);
    setAutoSaveSettings();
    return true;
}

void MainWindow::setupProgressWidgets()
{
    KPIM::ProgressStatusBarWidget * progressStatusBarWidget = new KPIM::ProgressStatusBarWidget(statusBar(), this);
    statusBar()->addPermanentWidget( progressStatusBarWidget->littleProgress(), 0 );
}

MainWindow::~MainWindow()
{
}

void MainWindow::saveProperties(KConfigGroup &config)
{
    if (!m_part)
        loadPart();

    m_part->saveProperties(config);
    config.writeEntry("docked", isHidden());
}

void MainWindow::readProperties(const KConfigGroup & config)
{
    if (!m_part)
        loadPart();
    m_part->readProperties(config);
    setVisible( !Settings::showTrayIcon() || !config.readEntry("docked", false) );
}

// TODO: move to part?
void MainWindow::optionsConfigureKeys()
{
    KShortcutsDialog dlg( KShortcutsEditor::AllActions,
      KShortcutsEditor::LetterShortcutsAllowed, this );

    dlg.addCollection(actionCollection());
    if (m_part)
        dlg.addCollection(m_part->actionCollection());

    dlg.configure();
}

// TODO: move to part?
void MainWindow::optionsConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config().data()->group( autoSaveGroup()) );
    KEditToolBar dlg(factory());
    connect(&dlg, SIGNAL(newToolBarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

// TODO: move to part?
void MainWindow::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group( autoSaveGroup()) );
}

KParts::BrowserExtension *MainWindow::browserExtension(KParts::ReadOnlyPart *p)
{
    return KParts::BrowserExtension::childObject( p );
}

void MainWindow::slotQuit()
{
    kapp->quit();
}

bool MainWindow::queryClose()
{
    if ( kapp->sessionSaving() ) {
        return true;
    }
    if ( !TrayIcon::getInstance() ) {
        delete m_part; // delete that here instead of dtor to ensure nested khtmlparts are deleted before singleton objects like KHTMLPageCache
        return true;
    }

    hide();
    return false;
}

void MainWindow::slotClearStatusText()
{
    m_statusLabel->setText(QString());
}

void MainWindow::slotSetStatusBarText( const QString & text )
{
    m_statusLabel->setText(text);
}

