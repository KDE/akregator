/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "app.h"
#include "akregator.h"
#include "akregator_part.h"

//settings

#include <dcopclient.h>

#include <ksqueezedtextlabel.h>
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
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kdebug.h>
#include <kparts/partmanager.h>

#include <qmetaobject.h>
#include <qpen.h>
#include <qpainter.h>
#include <private/qucomextra_p.h>


using namespace Akregator;

BrowserInterface::BrowserInterface( AkregatorMainWindow *shell, const char *name )
    : KParts::BrowserInterface( shell, name )
{
    m_shell = shell;
}

bool BrowserInterface::haveWindowLoaded() const
{
    return akreapp->haveWindowLoaded();
}

AkregatorMainWindow::AkregatorMainWindow()
    : KParts::MainWindow( 0L, "akregator_mainwindow" )
{
    // set the shell's ui resource file
    setXMLFile("akregator_shell.rc");

    m_browserIface=new BrowserInterface(this, "browser_interface");
    //m_activePart=0;
    m_part=0;

    m_manager = new KParts::PartManager(this, "akregator_partmanager");
    m_manager->setAllowNestedParts(true);
    
    connect(m_manager, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(createGUI(KParts::Part*)) );
    
    // then, setup our actions
    setupActions();

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

bool AkregatorMainWindow::loadPart()
{
    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory("libakregatorpart");
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        m_part = static_cast<aKregatorPart*>(factory->create(this, "akregator_part", "KParts::ReadOnlyPart" ));

        if (m_part)
        {
            // tell the KParts::MainWindow that this is indeed the main widget
            setCentralWidget(m_part->widget());

            connect(m_part, SIGNAL(setWindowCaption (const QString &)), this, SLOT(setCaption (const QString &)));
            //connect (m_part, SIGNAL(partChanged(KParts::ReadOnlyPart *)), this, SLOT(partChanged(KParts::ReadOnlyPart *)));
            connect( browserExtension(m_part), SIGNAL(loadingProgress(int)), this, SLOT(loadingProgress(int)) );
            //m_activePart=m_part;
            // and integrate the part's GUI with the shell's
            connectActionCollection(m_part->actionCollection());
            m_manager->addPart(m_part);
            //createGUI(m_part);
            browserExtension(m_part)->setBrowserInterface(m_browserIface);
            setAutoSaveSettings();
            return true;
        }
        return false;
    }
    else
    {
        KMessageBox::error(this, i18n("Could not find the aKregator part; please check your installation."));
        return false;
    }

}

void AkregatorMainWindow::loadStandardFile()
{
    show();
    QString file = KGlobal::dirs()->saveLocation("data", "akregator/data") + "/feeds.opml";
    
    if (!m_part)
        loadPart();
    m_part->openStandardFeedList();
}

AkregatorMainWindow::~AkregatorMainWindow()
{
    kdDebug() << "AkregatorMainWindow::~AkregatorMainWindow: delete part" << endl;
    delete m_part;
    kdDebug() << "AkregatorMainWindow::~AkregatorMainWindow: part deleted" << endl;
}

void AkregatorMainWindow::setCaption(const QString &a)
{
    if (sender() && (sender() == m_part) )
        KParts::MainWindow::setCaption(a);
}
/*
void AkregatorMainWindow::partChanged(KParts::ReadOnlyPart *p)
{
    //m_activePart=p;
    //createGUI(p);
}*/


void AkregatorMainWindow::addFeedToGroup(const QString& url, const QString& group)
{
    if (!m_part)
        loadPart();
    (static_cast<aKregatorPart*>(m_part))->addFeedToGroup( url, group );
}

void AkregatorMainWindow::setupActions()
{
    connectActionCollection(actionCollection());

    KStdAction::quit(kapp, SLOT(quit()), actionCollection());

    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();

    KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
}

void AkregatorMainWindow::saveProperties(KConfig* config)
{
    if (!m_part)
        loadPart();

    static_cast<Akregator::aKregatorPart*>(m_part)->saveProperties(config);
}

void AkregatorMainWindow::readProperties(KConfig* config)
{
    if (!m_part)
        loadPart();
    static_cast<Akregator::aKregatorPart*>(m_part)->readProperties(config);
}

void AkregatorMainWindow::fileNew()
{
    // this slot is called whenever the File->New menu is selected,
    // the New shortcut is pressed (usually CTRL+N) or the New toolbar
    // button is clicked

    // About this function, the style guide (
    // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
    // says that it should open a new window if the document is _not_
    // in its initial state.  This is what we do here..
//    if ( ! m_part->url().isEmpty() || m_part->isModified() )
 //   {
//	callObjectSlot( browserExtension(m_part), "saveSettings()", QVariant());

 //       aKregator *w=new aKregator();
 //	w->loadPart();
 //	w->show();
 //   }
}

void AkregatorMainWindow::optionsConfigureKeys()
{
    KKeyDialog dlg( true, this );

    dlg.insert(actionCollection());
    if (m_part)
        dlg.insert(m_part->actionCollection());

    dlg.configure();
}

void AkregatorMainWindow::optionsConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());

    // use the standard toolbar editor
    KEditToolbar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}



void AkregatorMainWindow::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void AkregatorMainWindow::fileOpen()
{
//    KURL url =
//        KFileDialog::getOpenURL( QString::null, QString::null, this );
    //
//    if (url.isEmpty() == false)
//    {
//        AkregatorMainWindow* newWin = new AkregatorMainWindow();
//        newWin->load( url );
//        newWin->show();
//    }
}

KParts::BrowserExtension *AkregatorMainWindow::browserExtension(KParts::ReadOnlyPart *p)
{
    return KParts::BrowserExtension::childObject( p );
}


// from konqmainwindow
void AkregatorMainWindow::connectActionCollection( KActionCollection *coll )
{
    if (!coll) return;
    connect( coll, SIGNAL( actionStatusText( const QString & ) ),
             this, SLOT( slotActionStatusText( const QString & ) ) );
    connect( coll, SIGNAL( clearStatusText() ),
             this, SLOT( slotClearStatusText() ) );
}

void AkregatorMainWindow::disconnectActionCollection( KActionCollection *coll )
{
    if (!coll) return;
    disconnect( coll, SIGNAL( actionStatusText( const QString & ) ),
                this, SLOT( slotActionStatusText( const QString & ) ) );
    disconnect( coll, SIGNAL( clearStatusText() ),
                this, SLOT( slotClearStatusText() ) );
}


bool AkregatorMainWindow::queryExit()
{
    if(m_part)
        static_cast<Akregator::aKregatorPart*>(m_part)->saveSettings();
    
    return KParts::MainWindow::queryExit();
}

bool AkregatorMainWindow::queryClose()
{
    if ( kapp->sessionSaving() || !m_part->isTrayIconEnabled() )
         return true;
    else
    {
        QPixmap shot = m_part->takeTrayIconScreenshot();

        // Associate source to image and show the dialog:
        QMimeSourceFactory::defaultFactory()->setPixmap("systray_shot", shot);
        KMessageBox::information(this, i18n( "<qt><p>Closing the main window will keep aKregator running in the system tray. Use 'Quit' from the 'File' menu to quit the application.</p><p><center><img source=\"systray_shot\"></center></p></qt>" ), i18n( "Docking in System Tray" ), "hideOnCloseInfo");
        hide();
    return false;
    }
}

// from KonqFrameStatusBar
void AkregatorMainWindow::fontChange(const QFont & /* oldFont */)
{
    int h = fontMetrics().height();
    if ( h < 13 ) h = 13;
    m_progressBar->setFixedHeight( h + 2 );

}

void AkregatorMainWindow::loadingProgress(int percent)
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

void AkregatorMainWindow::slotSetStatusBarText(const QString & s)
{
    m_permStatusText=s;
    m_statusLabel->setText(s);
}

void AkregatorMainWindow::slotActionStatusText(const QString &s)
{
    m_statusLabel->setText(s);
}

void AkregatorMainWindow::slotClearStatusText()
{
    m_statusLabel->setText(m_permStatusText);
}

// yanked from kdelibs
void AkregatorMainWindow::callObjectSlot( QObject *obj, const char *name, const QVariant &argument )
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

#include "akregator.moc"


// vim: set et ts=4 sts=4 sw=4:
