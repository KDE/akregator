/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "akregator_part.h"
#include "akregator_view.h"
#include "akregatorconfig.h"

#include <kparts/genericfactory.h>
#include <kinstance.h>
#include <kaction.h>
#include <kactionclasses.h>

#include <kstdaction.h>
#include <kfiledialog.h>

#include <qfile.h>

using namespace Akregator;

typedef KParts::GenericFactory< aKregatorPart > aKregatorFactory;
K_EXPORT_COMPONENT_FACTORY( libakregatorpart, aKregatorFactory )

aKregatorPart::aKregatorPart( QWidget *parentWidget, const char * /*widgetName*/,
                              QObject *parent, const char *name, const QStringList& )
    : KParts::ReadWritePart(parent, name)
{
    // we need an instance
    setInstance( aKregatorFactory::instance() );

    m_view=new aKregatorView(this, parentWidget, "Akregator View");

    // notify the part that this is our internal widget
    setWidget(m_view);

    // create our actions
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    KStdAction::save(this, SLOT(save()), actionCollection());

    /* -- ACTIONS */

    /* --- Feed popup menu */
    new KAction(i18n("&Add..."), "add", "Alt+Insert", m_view, SLOT(slotFeedAdd()), actionCollection(), "feed_add");
    new KAction(i18n("Add Feed &Group..."), "add", "Alt+Shift+Insert", m_view, SLOT(slotFeedAddGroup()), actionCollection(), "feed_add_group");
    new KAction(i18n("&Delete"), "delete", "Shift+Delete", m_view, SLOT(slotFeedRemove()), actionCollection(), "feed_remove");
    new KAction(i18n("&Modify"), "edit", "F2", m_view, SLOT(slotFeedModify()), actionCollection(), "feed_modify");
    new KAction(i18n("&Fetch"), "down", "Alt+Ctrl+F", m_view, SLOT(slotFetchCurrentFeed()), actionCollection(), "feed_fetch");
    new KAction(i18n("Fe&tch All"), "bottom", "Alt+Ctrl+A", m_view, SLOT(slotFetchAllFeeds()), actionCollection(), "feed_fetch_all");
    KRadioAction *ra=new KRadioAction(i18n("&Normal View"), "view_top_bottom", "Alt+Ctrl+1", m_view, SLOT(slotNormalView()), actionCollection(), "normal_view");
    ra->setExclusiveGroup( "ViewMode" );

    ra=new KRadioAction(i18n("&Widescreen View"), "view_left_right", "Alt+Ctrl+2", m_view, SLOT(slotWidescreenView()), actionCollection(), "widescreen_view");
    ra->setExclusiveGroup( "ViewMode" );

    ra=new KRadioAction(i18n("&Combined View"), "view_text", "Alt+Ctrl+3", m_view, SLOT(slotCombinedView()), actionCollection(), "combined_view");
    ra->setExclusiveGroup( "ViewMode" );

    // set our XML-UI resource file
    setXMLFile("akregator_part.rc");

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);
}



aKregatorPart::~aKregatorPart()
{
   Settings::writeConfig();
}

void aKregatorPart::setReadWrite(bool rw)
{
    ReadWritePart::setReadWrite(rw);
}

void aKregatorPart::setModified(bool modified)
{
    // get a handle on our Save action and make sure it is valid
    KAction *save = actionCollection()->action(KStdAction::stdName(KStdAction::Save));
    if (!save)
        return;

    // if so, we either enable or disable it based on the current
    // state
    if (modified)
        save->setEnabled(true);
    else
        save->setEnabled(false);

    // in any event, we want our parent to do it's thing
    ReadWritePart::setModified(modified);
}

void aKregatorPart::setStatusBar(const QString &text)
{
   emit setStatusBarText(text);
}


/*************************************************************************************************/
/* LOAD                                                                                          */
/*************************************************************************************************/

bool aKregatorPart::openFile()
{
    // m_file is always local so we can use QFile on it
    QFile file(m_file);
    if (file.open(IO_ReadOnly) == false)
        return false;

    // Read OPML feeds list and build QDom tree.
    QTextStream stream(&file);
    stream.setEncoding(QTextStream::UnicodeUTF8); // FIXME not all opmls are in utf8
    QDomDocument doc;
    QString str;

    str = stream.read();

    file.close();

    if (!doc.setContent(str))
        return false;

    if (!m_view->loadFeeds(doc)) // will take care of building feeds tree and loading archive
        return false;

    // just for fun, set the status bar
    setStatusBar( m_url.prettyURL() );

    return true;
}

/*************************************************************************************************/
/* SAVE                                                                                          */
/*************************************************************************************************/

bool aKregatorPart::saveFile()
{
    // if we aren't read-write, return immediately
    if (isReadWrite() == false)
        return false;

    // m_file is always local, so we use QFile
    QFile file(m_file);
    if (file.open(IO_WriteOnly) == false)
        return false;

    // use QTextStream to dump the text to the file
    QTextStream stream(&file);
    stream.setEncoding(QTextStream::UnicodeUTF8);

    // Write OPML data file.
    // Archive data files are saved elsewhere.

    QDomDocument newdoc;
    QDomElement root = newdoc.createElement( "opml" );
    root.setAttribute( "version", "1.0" );
    newdoc.appendChild( root );

    QDomElement head = newdoc.createElement( "head" );
    root.appendChild( head );

    QDomElement title = newdoc.createElement( "text" );
    head.appendChild( title );

    QDomText t = newdoc.createTextNode( "aKregator Feeds" );
    title.appendChild( t );

    QDomElement body = newdoc.createElement( "body" );
    root.appendChild( body );

    m_view->writeChildNodes( 0, body, newdoc);

    stream << newdoc.toString();

    file.close();

    return true;
}


/*************************************************************************************************/
/* SLOTS                                                                                         */
/*************************************************************************************************/

void aKregatorPart::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString file_name = KFileDialog::getOpenFileName( QString::null,
                        "*.opml|" + i18n("OPML Outlines (*.opml)") + "\n"
                        "*|" + i18n("All Files") );

    if (file_name.isEmpty() == false)
        openURL(file_name);
}

void aKregatorPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString file_name = KFileDialog::getSaveFileName( QString::null,
                        "*.opml|" + i18n("OPML Outlines (*.opml)") + "\n"
                        "*|" + i18n("All Files") );
    if (file_name.isEmpty() == false)
        saveAs(file_name);
}


/*************************************************************************************************/
/* STATIC METHODS                                                                                */
/*************************************************************************************************/

KAboutData* aKregatorPart::s_about = 0L;

KAboutData *aKregatorPart::createAboutData()
{
    if ( !s_about ) {
        s_about = new KAboutData("akregatorpart", I18N_NOOP("aKregatorPart"), "0.9",
                                 I18N_NOOP("This is a KPart for RSS aggregator"),
                                 KAboutData::License_GPL, "(C) 2004 Stanislav Karchebny", 0,
                                 "http://berk.upnet.ru/projects/kde/akregator",
                                 "Stanislav.Karchebny@kdemail.net");
        s_about->addAuthor("Stanislav Karchebny", I18N_NOOP("Author, Developer, Maintainer"),
                           "Stanislav.Karchebny@kdemail.net");
        s_about->addAuthor("Sashmit Bhaduri", I18N_NOOP("Developer"), "smt@vfemail.net");
    }
    return s_about;
}

#include "akregator_part.moc"
