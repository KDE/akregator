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

//settings

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kshortcutsdialog.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/partmanager.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <ktoolbar.h>
#include <kurl.h>

#include "progressdialog.h"
#include "statusbarprogresswidget.h"
#include "trayicon.h"

#include <QPixmap>

#include <Q3MimeSourceFactory>

namespace Akregator {

BrowserInterface::BrowserInterface( MainWindow *shell, const char *name )
    : KParts::BrowserInterface( shell)
{
    setObjectName(name);
    m_shell = shell;
}

MainWindow::MainWindow()
    : KParts::MainWindow()
{
    // set the shell's ui resource file
    setObjectName("akregator_mainwindow");
    setXMLFile("akregator_shell.rc");

    m_browserIface=new BrowserInterface(this, "browser_interface");

    m_part=0;

    // then, setup our actions

    toolBar()->show();
    // and a status bar
    statusBar()->show();

    int statH=fontMetrics().height()+2;
    m_statusLabel = new KSqueezedTextLabel(this);
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setSizePolicy(QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed ));
    m_statusLabel->setMinimumWidth( 0 );
    m_statusLabel->setFixedHeight( statH );
    statusBar()->addWidget( m_statusLabel, 1 );

    setupActions();
    createGUI(0L);
}

bool MainWindow::loadPart()
{
    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KPluginFactory *factory = KPluginLoader("akregatorpart").factory();
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<Akregator::Part*>( factory->create<KParts::ReadOnlyPart>(this) );

        if (m_part)
        {
            m_part->setObjectName("akregator_part");
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget(m_part->widget());

            connect(m_part, SIGNAL(setWindowCaption (const QString &)), this, SLOT(setCaption (const QString &)));

            connect(TrayIcon::getInstance(), SIGNAL(quitSelected()), this, SLOT(slotQuit()));
            // and integrate the part's GUI with the shell's
            createGUI(m_part);
            browserExtension(m_part)->setBrowserInterface(m_browserIface);
            setAutoSaveSettings();
            return true;
        }
        return false;
    }
    else
    {
        KMessageBox::error(this, i18n("Could not find the Akregator part; please check your installation."));
        return false;
    }

}

void MainWindow::setupProgressWidgets()
{
    KPIM::ProgressDialog *progressDialog = new KPIM::ProgressDialog( statusBar(), this );
    progressDialog->raise();
    progressDialog->hide();
    m_progressBar = new KPIM::StatusbarProgressWidget( progressDialog, statusBar() );
    m_progressBar->show();
    statusBar()->addPermanentWidget( m_progressBar, 0 );
}

MainWindow::~MainWindow()
{
}

void MainWindow::setCaption(const QString &a)
{
    KParts::MainWindow::setCaption(a);
}

void MainWindow::setupActions()
{
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();

    KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void MainWindow::saveProperties(KConfigGroup &config)
{
    if (!m_part)
        loadPart();

    static_cast<Akregator::Part*>(m_part)->saveProperties(config);
    config.writeEntry("docked", isHidden());

    //delete m_part;
}

void MainWindow::readProperties(const KConfigGroup & config)
{
    if (!m_part)
        loadPart();
    static_cast<Akregator::Part*>(m_part)->readProperties(config);

    if (Settings::showTrayIcon() && config.readEntry("docked", false))
        hide();
    else
        show();
}

void MainWindow::optionsConfigureKeys()
{
    KShortcutsDialog dlg( KShortcutsEditor::AllActions,
      KShortcutsEditor::LetterShortcutsDisallowed, this );

    dlg.addCollection(actionCollection());
    if (m_part)
        dlg.addCollection(m_part->actionCollection());

    dlg.configure();
}

void MainWindow::optionsConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config().data()->group( autoSaveGroup()) );

    // use the standard toolbar editor
    KEditToolBar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}



void MainWindow::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group( autoSaveGroup()) );
}


KParts::BrowserExtension *MainWindow::browserExtension(KParts::ReadOnlyPart *p)
{
    return KParts::BrowserExtension::childObject( p );
}

bool MainWindow::queryExit()
{
    kDebug() <<"MainWindow::queryExit()";
    if ( !kapp->sessionSaving() )
    {
        delete m_part; // delete that here instead of dtor to ensure nested khtmlparts are deleted before singleton objects like KHTMLPageCache
        m_part = 0;
    }
    else
        kDebug("MainWindow::queryExit(): saving session");

    return KMainWindow::queryExit();
}

void MainWindow::slotQuit()
{
    if (TrayIcon::getInstance())
        TrayIcon::getInstance()->hide();
    kapp->quit();
}

bool MainWindow::queryClose()
{
    if (kapp->sessionSaving() || TrayIcon::getInstance() == 0 || !TrayIcon::getInstance()->isVisible() )
    {
        return true;
    }
    else
    {
#ifdef __GNUC__
#warning takeScreenShot has to be reimplemented or removed
#endif
#if 0
        QPixmap shot = TrayIcon::getInstance()->takeScreenshot();

        // Associate source to image and show the dialog:
        Q3MimeSourceFactory::defaultFactory()->setPixmap("systray_shot", shot);
        KMessageBox::information(this, i18n( "<qt><p>Closing the main window will keep Akregator running in the system tray. Use 'Quit' from the 'File' menu to quit the application.</p><p><center><img source=\"systray_shot\" /></center></p></qt>" ), i18n( "Docking in System Tray" ), "hideOnCloseInfo");
#endif
        hide();
        return false;
    }
}


void MainWindow::slotClearStatusText()
{
    m_statusLabel->setText(QString());
}

void MainWindow::slotSetStatusBarText( const QString & text )
{
    m_statusLabel->setText(text);
}

} // namespace Akregator

#include "mainwindow.moc"


// vim: set et ts=4 sts=4 sw=4:
