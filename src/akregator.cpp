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

//settings
#include "settings_general.h"
#include "settings_browser.h"
#include "settings_archive.h"

#include <dcopclient.h>
#include <dcopobject.h>

#include <ksqueezedtextlabel.h>
#include <kkeydialog.h>
#include <kfiledialog.h>
#include <kprogress.h>
#include <kconfig.h>
#include <kurl.h>
#include <kconfigdialog.h>

#include <kedittoolbar.h>

#include <kaction.h>
#include <kstdaction.h>

#include <klibloader.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kdebug.h>

#include <qmetaobject.h>
#include <qpen.h>
#include <qpainter.h>
#include <private/qucomextra_p.h>
#include <akregator_part.h>

using namespace Akregator;

BrowserInterface::BrowserInterface( aKregator *shell, const char *name )
    : KParts::BrowserInterface( shell, name )
{
    m_shell = shell;
}

void BrowserInterface::updateUnread(int unread)
{
    m_shell->updateUnread(unread);
}

bool BrowserInterface::haveWindowLoaded() const
{
    return akreapp->haveWindowLoaded();
}

void BrowserInterface::newArticle(const QString& feed, const QPixmap&p, const QString& article)
{
    m_shell->newArticle(feed, p, article);

}

aKregator::aKregator()
    : KParts::MainWindow( 0L, "aKregator" ),
    m_quit(false)
{
    // set the shell's ui resource file
    setXMLFile("akregator_shell.rc");

    m_browserIface=new BrowserInterface(this, "browser_interface");
    m_activePart=0;
    m_part=0;

    // then, setup our actions
    setupActions();

    m_icon = new TrayIcon(this);

    if ( Settings::showTrayIcon() )
        m_icon->show();

    connect(m_icon, SIGNAL(quitSelected()),
            this, SLOT(quitProgram()));

    toolBar()->show();
    // and a status bar
    statusBar()->show();

    int statH=fontMetrics().height()+2;
    m_statusLabel = new KSqueezedTextLabel(this);
    m_statusLabel->setTextFormat(Qt::RichText);
    m_statusLabel->setSizePolicy(QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Fixed ));
    m_statusLabel->setMinimumWidth( 0 );
    m_statusLabel->setFixedHeight( statH );
    statusBar()->addWidget (m_statusLabel, 1, false);

    m_progressBar = new KProgress( this );
    // blame the following on KMLittleProgress
    m_progressBar->setMaximumWidth(fontMetrics().width( " 999.9 kB/s 00:00:01 " ) + 14);
    m_progressBar->setFixedHeight(statH);
    m_progressBar->hide();
    statusBar()->addWidget( m_progressBar, 0, true);
}

bool aKregator::loadPart()
{
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

    	    connect(m_part, SIGNAL(started(KIO::Job*)), this, SLOT(slotStarted(KIO::Job*)));
    	    connect(m_part, SIGNAL(completed()), this, SLOT(slotCompleted()));
    	    connect(m_part, SIGNAL(canceled(const QString&)), this, SLOT(slotCanceled(const QString &)));
    	    connect(m_part, SIGNAL(completed(bool)), this, SLOT(slotCompleted()));

    	    connect(m_part, SIGNAL(setWindowCaption (const QString &)), this, SLOT(setCaption (const QString &)));
            connect (m_part, SIGNAL(partChanged(KParts::ReadOnlyPart *)), this, SLOT(partChanged(KParts::ReadOnlyPart *)));
            connect( browserExtension(m_part), SIGNAL(loadingProgress(int)), this, SLOT(loadingProgress(int)) );
            m_activePart=m_part;
            // and integrate the part's GUI with the shell's
            connectActionCollection(m_part->actionCollection());
            createGUI(m_part);
            browserExtension(m_part)->setBrowserInterface(m_browserIface);
            setAutoSaveSettings();
        }
        return true;
    }
    else
    {
        KMessageBox::error(this, i18n("Could not find the aKregator part; please check your installation."));
        return false;
    }

}

void aKregator::loadStandardFile()
{
   show();
   QString file=KGlobal::dirs()->saveLocation("data", "akregator/data") + "/feeds.opml";
   load(file);
}

aKregator::~aKregator()
{}

void aKregator::setCaption(const QString &a)
{
    if (sender() && (sender() == m_part) )
        KParts::MainWindow::setCaption(a);
}

void aKregator::partChanged(KParts::ReadOnlyPart *p)
{
    m_activePart=p;
    createGUI(p);
}

void aKregator::load(const KURL& url)
{
    if (!m_part)
	    loadPart();
    m_part->openURL( url );
}

void aKregator::addFeedToGroup(const QString& url, const QString& group)
{
    if (!m_part)
        loadPart();
    (static_cast<aKregatorPart*>(m_part))->addFeedToGroup( url, group );
}

void aKregator::setupActions()
{
    connectActionCollection(actionCollection());

    KStdAction::quit(this, SLOT(quitProgram()), actionCollection());

    m_stopAction = new KAction( i18n( "&Stop" ), "stop", Key_Escape, this, SLOT( slotStop() ), actionCollection(), "stop" );
    m_stopAction->setEnabled(false);

/*    m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
    m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());
*/
    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(showOptions()), actionCollection());
}

void aKregator::saveProperties(KConfig* config)
{
    if (!m_part)
        loadPart();

    static_cast<Akregator::aKregatorPart*>(m_part)->saveProperties(config);
}

void aKregator::readProperties(KConfig* config)
{
    if (!m_part) // if blank url, load part anyways
        loadPart();
    static_cast<Akregator::aKregatorPart*>(m_part)->readProperties(config);
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
	callObjectSlot( browserExtension(m_part), "saveSettings()", QVariant());

        aKregator *w=new aKregator();
	w->loadPart();
	w->show();
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
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());

    // use the standard toolbar editor
    KEditToolbar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

void aKregator::showOptions()
{
    if ( KConfigDialog::showDialog( "settings" ) )
        return;

    KConfigDialog *dialog = new KConfigDialog( this, "settings", Settings::self() );
    dialog->addPage(new settings_general(0, "General"), i18n("General"), "package_settings");
    dialog->addPage(new settings_archive(0, "Archive"), i18n("Archive"), "package_settings");
    dialog->addPage(new settings_browser(0, "Browser"), i18n("Browser"), "package_network");
    connect( dialog, SIGNAL(settingsChanged()),
             m_part, SLOT(saveSettings()) );
    connect( dialog, SIGNAL(settingsChanged()),
             m_icon, SLOT(settingsChanged()) );

    dialog->show();
}

void aKregator::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void aKregator::fileOpen()
{
    KURL url =
        KFileDialog::getOpenURL( QString::null, QString::null, this );

    if (url.isEmpty() == false)
    {
        aKregator* newWin = new aKregator;
        newWin->load( url );
        newWin->show();
    }
}

KParts::BrowserExtension *aKregator::browserExtension(KParts::ReadOnlyPart *p)
{
    return KParts::BrowserExtension::childObject( p );
}


// from konqmainwindow
void aKregator::connectActionCollection( KActionCollection *coll )
{
    if (!coll) return;
    connect( coll, SIGNAL( actionStatusText( const QString & ) ),
             this, SLOT( slotActionStatusText( const QString & ) ) );
    connect( coll, SIGNAL( clearStatusText() ),
             this, SLOT( slotClearStatusText() ) );
}

void aKregator::disconnectActionCollection( KActionCollection *coll )
{
    if (!coll) return;
    disconnect( coll, SIGNAL( actionStatusText( const QString & ) ),
                this, SLOT( slotActionStatusText( const QString & ) ) );
    disconnect( coll, SIGNAL( clearStatusText() ),
                this, SLOT( slotClearStatusText() ) );
}


bool aKregator::queryExit()
{
    if(!m_part)
        return KParts::MainWindow::queryExit();

    if( Settings::markAllFeedsReadOnExit() )
        emit markAllFeedsRead();


    Settings::writeConfig();
    return KParts::MainWindow::queryExit();
}

bool aKregator::queryClose()
{
    if ( kapp->sessionSaving() || m_quit || !Settings::showTrayIcon() )
        return m_part->queryClose();
    else
    {
        // Compute size and position of the pixmap to be grabbed:
        QPoint g = m_icon->mapToGlobal(m_icon->pos());
        int desktopWidth  = kapp->desktop()->width();
        int desktopHeight = kapp->desktop()->height();
        int tw = m_icon->width();
        int th = m_icon->height();
        int w = desktopWidth / 4;
        int h = desktopHeight / 9;
        int x = g.x() + tw/2 - w/2; // Center the rectange in the systray icon
        int y = g.y() + th/2 - h/2;
        if (x < 0)                 x = 0; // Move the rectangle to stay in the desktop limits
        if (y < 0)                 y = 0;
        if (x + w > desktopWidth)  x = desktopWidth - w;
        if (y + h > desktopHeight) y = desktopHeight - h;

        // Grab the desktop and draw a circle arround the icon:
        QPixmap shot = QPixmap::grabWindow(qt_xrootwin(), x, y, w, h);
        QPainter painter(&shot);
        const int MARGINS = 6;
        const int WIDTH   = 3;
        int ax = g.x() - x - MARGINS -1;
        int ay = g.y() - y - MARGINS -1;
        painter.setPen( QPen(Qt::red/*KApplication::palette().active().highlight()*/, WIDTH) );
        painter.drawArc(ax, ay, tw + 2*MARGINS, th + 2*MARGINS, 0, 16*360);
        painter.end();

        // Associate source to image and show the dialog:
        QMimeSourceFactory::defaultFactory()->setPixmap("systray_shot", shot);
        KMessageBox::information(this, i18n( "<qt>Closing the main window will keep aKregator running in the system tray. Use 'Quit' from the 'File' menu to quit the application.<p><p><center><img source=\"systray_shot\"></center></p></qt>" ), i18n( "Docking in System Tray" ), "hideOnCloseInfo");
        hide();
    }
    return false;

}
void aKregator::quitProgram()
{
     static_cast<Akregator::aKregatorPart*>(m_part)->saveSettings();
     m_quit = true;
     close();
     m_quit = false;
}

// from KonqFrameStatusBar
void aKregator::fontChange(const QFont & /* oldFont */)
{
    int h = fontMetrics().height();
    if ( h < 13 ) h = 13;
    m_progressBar->setFixedHeight( h + 2 );

}

void aKregator::updateUnread(int unread)
{
    m_icon->updateUnread(unread);
}

void aKregator::newArticle(const QString& feed, const QPixmap&p, const QString& article)
{
    m_icon->newArticle(feed, p, article);
}

void aKregator::loadingProgress(int percent)
{
    if ( percent > -1 && percent < 100 )
    {
        if ( !m_progressBar->isVisible() )
            m_progressBar->show();
    }
    else
        m_progressBar->hide();

    m_progressBar->setValue( percent );
}

void aKregator::slotSetStatusBarText(const QString & s)
{
    m_permStatusText=s;
    m_statusLabel->setText(s);
}

void aKregator::slotActionStatusText(const QString &s)
{
    m_statusLabel->setText(s);
}

void aKregator::slotClearStatusText()
{
    m_statusLabel->setText(m_permStatusText);
}


void aKregator::slotStop()
{
    m_activePart->closeURL();
}

// yanked from kdelibs
void aKregator::callObjectSlot( QObject *obj, const char *name, const QVariant &argument )
{
    if (!obj)
	    return;

    int slot = obj->metaObject()->findSlot( name );

    QUObject o[ 2 ];
    QStringList strLst;
    uint i;

    switch ( argument.type() )
    {
        case QVariant::Invalid:
            break;
        case QVariant::String:
            static_QUType_QString.set( o + 1, argument.toString() );
            break;
        case QVariant::StringList:
            strLst = argument.toStringList();
            static_QUType_ptr.set( o + 1, &strLst );
            break;
        case QVariant::Int:
            static_QUType_int.set( o + 1, argument.toInt() );
            break;
        case QVariant::UInt:
            i = argument.toUInt();
            static_QUType_ptr.set( o + 1, &i );
            break;
        case QVariant::Bool:
            static_QUType_bool.set( o + 1, argument.toBool() );
            break;
        default: return;
    }

    obj->qt_invoke( slot, o );
}

void aKregator::slotStarted(KIO::Job *)
{
    m_stopAction->setEnabled(true);
}

void aKregator::slotCanceled(const QString &)
{
    m_stopAction->setEnabled(false);
}

void aKregator::slotCompleted()
{
    m_stopAction->setEnabled(false);
}

#include "akregator.moc"


// vim: set et ts=4 sts=4 sw=4:
