/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#include "akregator_part.h"
#include "addfeeddialog.h"
#include "feedstree.h"
#include "articlelist.h"
#include "feed.h"

#include <ktrader.h>
#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <klistview.h>
#include <khtml_part.h>
#include <kdebug.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qvaluevector.h>
#include <qtabwidget.h>//??
#include <qgrid.h>//??

using namespace Akregator;

aKregatorPart::aKregatorPart( QWidget *parentWidget, const char */*widgetName*/,
                                  QObject *parent, const char *name )
    : KParts::ReadWritePart(parent, name)
    , m_feeds()
    , m_html(0)
{
    // we need an instance
    setInstance( aKregatorPartFactory::instance() );

    m_panner1Sep << 1 << 1;
    m_panner2Sep << 1 << 1;

    m_panner1 = new QSplitter(QSplitter::Horizontal, parentWidget, "panner1");
    m_panner1->setOpaqueResize( true );

    m_tree = new FeedsTree( m_panner1, "FeedsTree" );

    connect(m_tree, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
            this, SLOT(slotContextMenu(KListView*, QListViewItem*, const QPoint&)));
    connect(m_tree, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(slotItemChanged(QListViewItem*)));
    connect(m_tree, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(slotItemChanged(QListViewItem*)));

    m_panner1->setResizeMode( m_tree, QSplitter::KeepSize );

    //hmm?? FIXME make it a object field
    QTabWidget *tabs = new QTabWidget(m_panner1);
    QWhatsThis::add(tabs, i18n("You can view multiple articles in several open tabs."));

    QGrid *w1 = new QGrid(1, parentWidget);
    QWhatsThis::add(w1, i18n("Articles list."));

    m_panner2 = new QSplitter(QSplitter::Vertical, w1, "panner2");

    m_articles = new ArticleList( m_panner2, "articles" );
    connect( m_articles, SIGNAL(clicked(QListViewItem *)), this, SLOT( slotArticleSelected(QListViewItem *)) );

    m_panner1->setSizes( m_panner1Sep );
    m_panner2->setSizes( m_panner2Sep );

    // We require KHTMLPart here because of begin()/write()/end() later, as I don't know if there are
    // any text/html parts that implement this interface. If anyone complains, I'll think about fixing it.
    KTrader::OfferList offers = KTrader::self()->query("text/html", "'KParts/ReadOnlyPart' in ServiceTypes");

    KLibFactory *factory = 0;
    // in theory, we only care about the first one.. but let's try all
    // offers just in case the first can't be loaded for some reason
    KTrader::OfferList::Iterator it(offers.begin());
    for( ; it != offers.end(); ++it)
    {
        KService::Ptr ptr = (*it);

        // we now know that our offer can handle HTML and is a part.
        // since it is a part, it must also have a library... let's try to
        // load that now
        factory = KLibLoader::self()->factory( ptr->library() );
        if (factory)
        {
            m_html = static_cast<KHTMLPart *>(factory->create(m_panner2, ptr->name(), "KParts::ReadOnlyPart"));
            if (!m_html) continue; // its not KHTMLPart, keep looking
            break;
        }
    }

    if (!m_html)
    {
        KMessageBox::error(parentWidget, i18n("Could not find a suitable HTML component"));
        ::exit(-1);
    }
    //end KHTML part

    QWhatsThis::add(m_html->widget(), i18n("Browsing area."));

    tabs->addTab(w1, "Articles");

    // notify the part that this is our internal widget
    setWidget(m_panner1);

    // create our actions
    KStdAction::open(this, SLOT(fileOpen()), actionCollection());
    KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
    KStdAction::save(this, SLOT(save()), actionCollection());

    /* -- ACTIONS */

    /* --- Feed popup menu */
    new KAction(i18n("&Add"), "", "Alt+Insert", this, SLOT(slotFeedAdd()), actionCollection(), "feed_add");
    new KAction(i18n("Add feed &group"), "", "Alt+Shift+Insert", this, SLOT(slotFeedAddGroup()), actionCollection(), "feed_add_group");
    new KAction(i18n("&Delete"), "", "Shift+Delete", this, SLOT(slotFeedRemove()), actionCollection(), "feed_remove");
    new KAction(i18n("&Modify"), "", "F2", this, SLOT(slotFeedModify()), actionCollection(), "feed_modify");
    new KAction(i18n("&Copy"), "", "Alt+Ctrl+C", this, SLOT(slotFeedCopy()), actionCollection(), "feed_copy");
    new KAction(i18n("&Fetch"), "down", "Alt+Ctrl+F", this, SLOT(slotFetchCurrentFeed()), actionCollection(), "feed_fetch");
    new KAction(i18n("Fe&tch all"), "bottom", "Alt+Ctrl+A", this, SLOT(slotFetchAllFeeds()), actionCollection(), "feed_fetch_all");

    // set our XML-UI resource file
    setXMLFile("akregator_part.rc");

    // -- DEFAULT INIT
    // Root item (will be reset when loading from file)
    KListViewItem *elt = new KListViewItem( m_tree, i18n("All Feeds") );
    m_feeds.addFeedGroup(elt);

    m_html->openURL( ::locate( "data", "akregatorpart/welcome.html" ) );
    // -- /DEFAULT INIT

    // we are read-write by default
    setReadWrite(true);

    // we are not modified since we haven't done anything yet
    setModified(false);
}

// clears everything out, even removes DEFAULT INIT
void aKregatorPart::reset()
{
    m_feeds.clearFeeds();
    m_tree->clear();
    setModified(false);
}

aKregatorPart::~aKregatorPart()
{
}

void aKregatorPart::setReadWrite(bool rw)
{
    // notify your internal widget of the read-write state
/*    m_tree->setReadOnly(!rw);

    if (rw)
        connect(m_widget, SIGNAL(textChanged()),
                this,     SLOT(setModified()));
    else
    {
        disconnect(m_widget, SIGNAL(textChanged()),
                   this,     SLOT(setModified()));
    }*/

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

bool aKregatorPart::openFile()
{
    // m_file is always local so we can use QFile on it
    QFile file(m_file);
    if (file.open(IO_ReadOnly) == false)
        return false;

    // Read OPML feeds list and build QDom tree.
    QTextStream stream(&file);
    QDomDocument doc;
    QString str;

    str = stream.read();

    file.close();

    if (!doc.setContent(str))
        return false;

    if (!loadFeeds(doc)) // will take care of building feeds tree and loading archive
        return false;

    // just for fun, set the status bar
    emit setStatusBarText( m_url.prettyURL() );

    return true;
}

// oh ugly as hell (move to one of feed ctors? how to connect signal then, pass a KPart *?)
Feed *aKregatorPart::addFeed_Internal(QListViewItem *elt, QString title, QString xmlUrl, QString htmlUrl, QString description, bool isLiveJournal, QString ljUserName, Feed::LJAuthMode ljAuthMode, QString ljLogin, QString ljPassword, bool updateTitle)
{
    m_feeds.addFeed(elt);

    Feed *feed = static_cast<Feed *>(m_feeds.find(elt));

    feed->title          = title;
    feed->xmlUrl         = xmlUrl;
    feed->htmlUrl        = htmlUrl;
    feed->description    = description;
    feed->isLiveJournal  = isLiveJournal;
    feed->ljUserName     = ljUserName;
    feed->ljAuthMode     = ljAuthMode;
    feed->ljLogin        = ljLogin;
    feed->ljPassword     = ljPassword;
    feed->updateTitle    = updateTitle;

    connect( feed, SIGNAL(fetched(Feed* )), this, SLOT(slotFeedFetched(Feed *)) );

    return feed;
}

void aKregatorPart::parseChildNodes(QDomNode &node, KListViewItem *parent)
{
    QDomElement e = node.toElement(); // try to convert the node to an element.
    if( !e.isNull() )
    {
        KListViewItem *elt;
        if (parent)
            elt = new KListViewItem( parent, m_tree->lastItem(), e.attribute("text") );
        else
            elt = new KListViewItem( m_tree, m_tree->lastItem(), e.attribute("text") );

        if (e.hasAttribute("htmlUrl"))
        {
            addFeed_Internal( elt,
                              e.attribute("title"),
                              e.attribute("xmlUrl"),
                              e.attribute("htmlUrl"),
                              e.attribute("description"),
                              e.attribute("isLiveJournal") == "true" ? true : false,
                              e.attribute("ljUserName"),
                              Feed::authModeFromString( e.attribute("ljAuthMode", "none") ),
                              e.attribute("ljLogin"),
                              e.attribute("ljPassword"),
                              e.attribute("updateTitle") == "true" ? true : false
                            );
        }
        else
            m_feeds.addFeedGroup(elt);

        if (e.hasChildNodes())
        {
            QDomNode child = e.firstChild();
            while(!child.isNull())
            {
                parseChildNodes(child, elt);
                child = child.nextSibling();
            }
        }
    }
}

bool aKregatorPart::loadFeeds(const QDomDocument& doc)
{
    // this should be OPML document
    QDomElement root = doc.documentElement();
    if (root.tagName().lower() != "opml")
        return false;

    // we ignore <head> and only parse <body> part
    QDomNodeList list = root.elementsByTagName("body");
    if (list.count() != 1)
        return false;

    QDomElement body = list.item(0).toElement();
    if (body.isNull())
        return false;

    reset();

    QDomNode n = body.firstChild();
    while( !n.isNull() )
    {
        parseChildNodes(n);
        n = n.nextSibling();
    }

    return true;
}

bool aKregatorPart::saveFile() // TODO: rewrite using QDom* classes
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

    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl
           << "<opml version=\"1.0\">"                      << endl
           << "<head>"                                      << endl
           << "   <title>aKregator Feeds</title>"           << endl
//           << "   <ownerName></ownerName>"                  << endl
//           << "   <ownerEmail></ownerEmail>"                << endl
           << "</head>"                                     << endl
           << "<body>"                                      << endl;

    QValueVector<int> closeDepths; // at what depth we should close exterior outline

    for (QListViewItemIterator it(m_tree); it.current(); it++)
    {
        FeedGroup *feed = m_feeds.find(*it);

        if (feed->isGroup())
        {
            if ((*it)->childCount() > 0)
            {
                // only open outline, will be closed later
                closeDepths.push_back( (*it)->depth() );

                stream << "<outline text=\"" << (*it)->text(0) << "\">" << endl;
            }
            else
            {
                // empty outline, write at once
                stream << "<outline text=\"" << (*it)->text(0) << "\"></outline>" << endl;
            }
        }
        else
        {
            // close outlines, if any, above current depth
            if (!closeDepths.empty() && (*it)->depth() <= closeDepths.last())
            {
                closeDepths.pop_back();
                stream << "</outline>" << endl;
            }

            feed->save(stream, (*it)->depth());
        }
    }

    while( closeDepths.size() > 0 )
    {
        closeDepths.pop_back();
        stream << "</outline>" << endl;
    }

    stream << "</body>"                                     << endl
           << "</opml>"                                     << endl;

    file.close();

    return true;
}

void aKregatorPart::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    QString file_name = KFileDialog::getOpenFileName();

    if (file_name.isEmpty() == false)
        openURL(file_name);
}

void aKregatorPart::fileSaveAs()
{
    // this slot is called whenever the File->Save As menu is selected,
    QString file_name = KFileDialog::getSaveFileName();
    if (file_name.isEmpty() == false)
        saveAs(file_name);
}

void aKregatorPart::slotContextMenu(KListView*, QListViewItem*, const QPoint& p)
{
   QWidget *w = factory()->container("feeds_popup", this);
   if (w)
      static_cast<QPopupMenu *>(w)->exec(p);
}

void aKregatorPart::slotItemChanged(QListViewItem *item)
{
    FeedGroup *feed = static_cast<FeedGroup *>(m_feeds.find(item));

    if (feed->isGroup())
    {
        actionCollection()->action("feed_add")->setEnabled(true);
        actionCollection()->action("feed_add_group")->setEnabled(true);
    }
    else
    {
        actionCollection()->action("feed_add")->setEnabled(false);
        actionCollection()->action("feed_add_group")->setEnabled(false);

        slotUpdateArticleList( static_cast<Feed *>(feed) );
    }

    if (item->parent())
        actionCollection()->action("feed_remove")->setEnabled(true);
    else
        actionCollection()->action("feed_remove")->setEnabled(false);

}

void aKregatorPart::slotUpdateArticleList(Feed *source)
{
    m_articles->setUpdatesEnabled(false);
    m_articles->clear(); // FIXME could become rather slow if we store a lot of archive items?

    kdDebug() << "Fetched " << source->articles.count() << " articles\n";

    if (source->articles.count() > 0)
    {
        Article::List::const_iterator it;
        Article::List::const_iterator end = source->articles.end();
        for (it = source->articles.begin(); it != end; ++it)
        {
            kdDebug() << "[adding] Article entitled " << (*it).title() << " with contents " << (*it).description() << endl;
            new KListViewItem( m_articles, m_articles->lastItem(), (*it).title() );
        }
    }
    m_articles->setUpdatesEnabled(true);
    m_articles->triggerUpdate();
}

// code below "borrowed" from KnowIt
// NOTE: feed can only be added to a feed group as a child
void aKregatorPart::slotFeedAdd()
{
    if (!m_tree->currentItem() || m_feeds.find(m_tree->currentItem())->isGroup() == false)
    {
        KMessageBox::error(widget(), i18n("You have to choose feed group before adding feed."));
        return;
    }

    KListViewItem *elt;
    AddFeedDialog *dlg = new AddFeedDialog( this->widget(), "add_feed" );
    if (dlg->exec() != QDialog::Accepted) return;

    QString text = dlg->feedNameEdit->text();

    QListViewItem *lastChild = m_tree->currentItem()->firstChild();
    while (lastChild && lastChild->nextSibling())
        lastChild = lastChild->nextSibling();

    if (lastChild)
        elt = new KListViewItem(m_tree->currentItem(), lastChild, text);
    else
        elt = new KListViewItem(m_tree->currentItem(), text);

    addFeed_Internal( elt,
                      text,
                      dlg->urlEdit->text(),
                      "",
                      "",
                      dlg->ljUserChkbox->isChecked(),
                      dlg->ljUserEdit->text(),
                      dlg->ljAuthMode->selectedId() == 0 ? Feed::AuthNone
                    : dlg->ljAuthMode->selectedId() == 1 ? Feed::AuthGlobal
                    : dlg->ljAuthMode->selectedId() == 2 ? Feed::AuthLocal
                    : Feed::AuthNone,
                      dlg->loginEdit->text(),
                      dlg->passwordEdit->text(),
                      dlg->nameFromRssChkbox->isChecked()
                    );
}

void aKregatorPart::slotFeedAddGroup()
{
    if (!m_tree->currentItem() || m_feeds.find(m_tree->currentItem())->isGroup() == false)
    {
        KMessageBox::error(widget(), i18n("You have to choose feed group before adding subgroup."));
        return;
    }

    bool Ok;
    KListViewItem *elt;

    QString text = KInputDialog::getText(i18n("Add feed group"), i18n("Feed group title:"), "", &Ok);
    if (!Ok) return;

    QListViewItem *lastChild = m_tree->currentItem()->firstChild();
    while (lastChild && lastChild->nextSibling())
        lastChild = lastChild->nextSibling();

    if (lastChild)
        elt = new KListViewItem(m_tree->currentItem(), lastChild, text);
    else
        elt = new KListViewItem(m_tree->currentItem(), text);

    m_feeds.addFeedGroup(elt);
}

void aKregatorPart::slotFeedRemove()
{
    QListViewItem *elt = m_tree->currentItem();
    if (!elt) return;
    QListViewItem *parent = elt->parent();
    if (!parent) return; // don't delete root element! (safety valve)

    QString msg = elt->childCount() ?
        i18n("<qt>Are you sure you want to delete group<br><b>%1</b><br> and its subgroups and feeds?</qt>") :
        i18n("<qt>Are you sure you want to delete feed<br><b>%1</b>?</qt>");
    if (KMessageBox::questionYesNo(0, msg.arg(elt->text(0))) == KMessageBox::Yes)
    {
        m_feeds.removeFeed(elt);
/*        if (!Notes.count())
            slotActionUpdate();
        if (!parent)
            parent = Items->firstChild();
        Items->prevItem = 0;
        slotNoteChanged(parent);*/
    }
}

void aKregatorPart::slotFeedModify()
{
/*
    AddFeedDialog *dlg = new AddFeedDialog( this->widget(), "edit_feed" );
    Feed *feed = static_cast<Feed *>(m_feeds.find(elt));

    feed->title          = dlg->feedNameEdit->text();
    feed->xmlUrl         = dlg->urlEdit->text();
    feed->isLiveJournal  = dlg->ljUserChkbox->isChecked();
    feed->ljUserName     = dlg->ljUserEdit->text();
    feed->ljAuthMode     = dlg->ljAuthMode->selectedId() == 0 ? Feed::AuthNone
                         : dlg->ljAuthMode->selectedId() == 1 ? Feed::AuthGlobal
                         : dlg->ljAuthMode->selectedId() == 2 ? Feed::AuthLocal
                         : Feed::AuthNone;
    feed->ljLogin        = dlg->loginEdit->text();
    feed->ljPassword     = dlg->passwordEdit->text();
    feed->updateTitle    = dlg->nameFromRssChkbox->isChecked();

    if (dlg->exec() != QDialog::Accepted) return;

    feed->title          = dlg->feedNameEdit->text();
    feed->xmlUrl         = dlg->urlEdit->text();
    feed->isLiveJournal  = dlg->ljUserChkbox->isChecked();
    feed->ljUserName     = dlg->ljUserEdit->text();
    feed->ljAuthMode     = dlg->ljAuthMode->selectedId() == 0 ? Feed::AuthNone
                         : dlg->ljAuthMode->selectedId() == 1 ? Feed::AuthGlobal
                         : dlg->ljAuthMode->selectedId() == 2 ? Feed::AuthLocal
                         : Feed::AuthNone;
    feed->ljLogin        = dlg->loginEdit->text();
    feed->ljPassword     = dlg->passwordEdit->text();
    feed->updateTitle    = dlg->nameFromRssChkbox->isChecked();
*/
}

void aKregatorPart::slotFeedCopy()
{
}

void aKregatorPart::slotFetchCurrentFeed()
{
    if (m_tree->currentItem())
    {
        Feed *f = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
        if (f && !f->isGroup())
        {
            kdDebug() << "Fetching item " << f->title << endl;
            f->fetch();
        }
        else // its a feed group, need to fetch from all its children, recursively
        {
            FeedGroup *g = m_feeds.find(m_tree->currentItem());
            if (!g) {
                KMessageBox::error( widget(), "Internal error, feeds tree inconsistent!" );
                return;
            }

            for (QListViewItemIterator it(m_tree->currentItem()); it.current(); ++it)
            {
                kdDebug() << "Fetching subitem " << (*it)->text(0) << endl;
                Feed *f = static_cast<Feed *>(m_feeds.find(*it));
                if (f && !f->isGroup())
                    f->fetch();
            }
        }
    }
}

void aKregatorPart::slotFetchAllFeeds()
{
    for (QListViewItemIterator it(m_tree->firstChild()); it.current(); ++it)
    {
        kdDebug() << "Fetching subitem " << (*it)->text(0) << endl;
        Feed *f = static_cast<Feed *>(m_feeds.find(*it));
        if (f && !f->isGroup())
            f->fetch();
    }
}


void aKregatorPart::slotFeedFetched(Feed *feed)
{
    // Feed finished fetching
    // If its a currenly selected feed, update view
    kdDebug() << k_funcinfo << "BEGIN" << endl;
    if (feed->item() == m_tree->currentItem())
    {
        kdDebug() << k_funcinfo << "Updating article list" << endl;
        slotUpdateArticleList(feed);
    }

    // Also, update unread counts


    kdDebug() << k_funcinfo << "END" << endl;
}


void aKregatorPart::slotArticleSelected(QListViewItem *item)
{
    kdDebug() << k_funcinfo << "BEGIN" << endl;
    Feed *feed = static_cast<Feed *>(m_feeds.find(m_tree->currentItem()));
    if (!feed) return;

    int index = m_articles->itemIndex(item);
    kdDebug() << k_funcinfo << "Selected item number " << index << endl;

    if (index < 0 || static_cast<Article::List::size_type>(index) > feed->articles.size())
        return;

    m_html->begin( KURL( "file:/tmp/something.html" ) );
    // TODO add KMail-like article summary above the article.
    m_html->write( feed->articles[index].description() );
    m_html->end();

    kdDebug() << k_funcinfo << "END" << endl;
}


// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance*  aKregatorPartFactory::s_instance = 0L;
KAboutData* aKregatorPartFactory::s_about = 0L;

aKregatorPartFactory::aKregatorPartFactory()
    : KParts::Factory()
{
}

aKregatorPartFactory::~aKregatorPartFactory()
{
    delete s_instance;
    delete s_about;

    s_instance = 0L;
}

KParts::Part* aKregatorPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
                                                        QObject *parent, const char *name,
                                                        const char *classname, const QStringList &/*args*/ )
{
    // Create an instance of our Part
    aKregatorPart* obj = new aKregatorPart( parentWidget, widgetName, parent, name );

    // See if we are to be read-write or not
    if (QCString(classname) == "KParts::ReadOnlyPart")
        obj->setReadWrite(false);

    return obj;
}

KInstance* aKregatorPartFactory::instance()
{
    if( !s_instance )
    {
        s_about = new KAboutData("akregatorpart", I18N_NOOP("aKregatorPart"), "0.9");
        s_about->addAuthor("Stanislav Karchebny", 0, "Stanislav.Karchebny@kdemail.net");
        s_instance = new KInstance(s_about);
    }
    return s_instance;
}

extern "C"
{
    void* init_libakregatorpart()
    {
        return new aKregatorPartFactory;
    }
};

#include "akregator_part.moc"
