/***************************************************************************
 *   Copyright (C) 2004 by Stanislav Karchebny                             *
 *   Stanislav.Karchebny@kdemail.net                                       *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef _AKREGATORPART_H_
#define _AKREGATORPART_H_

#include "feed.h"
#include "feedscollection.h"
#include <kparts/part.h>
#include <kparts/factory.h>
#include <qsplitter.h>

class QWidget;
class QPainter;
class QMultiLineEdit;
class QDomDocument;
class QDomElement;
class QListViewItem;
class KListViewItem;
class KURL;
class KListView;
class KHTMLPart;

namespace Akregator {
    class FeedsTree;
    class ArticleList;
    class ArticleViewer;
}

using namespace Akregator;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 */
class aKregatorPart : public KParts::ReadWritePart
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    aKregatorPart(QWidget *parentWidget, const char *widgetName,
                    QObject *parent, const char *name);

    /**
     * Destructor
     */
    virtual ~aKregatorPart();

    /**
     * This is a virtual function inherited from KParts::ReadWritePart.
     * A shell will use this to inform this Part if it should act
     * read-only
     */
    virtual void setReadWrite(bool rw);

    /**
     * Reimplemented to disable and enable Save action
     */
    virtual void setModified(bool modified);

    void reset();

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();

    /**
     * This must be implemented by each read-write part
     */
    virtual bool saveFile();

    /**
     * Parse OPML presentation of feeds and read in articles archive, if present.
     * @param doc QDomDocument generated from OPML by openFile().
     */
    bool loadFeeds(const QDomDocument& doc);
    void parseChildNodes(QDomNode &node, KListViewItem *parent = 0);

    void writeChildNodes( QListViewItem *item, QDomElement &node, QDomDocument &document );

public slots:

protected slots:
    /** Current item has changed */
    void slotItemChanged(QListViewItem*);
    /** Shows requested popup menu */
    void slotContextMenu(KListView*, QListViewItem*, const QPoint&);

    void slotFeedFetched(Feed *);

    void slotUpdateArticleList(Feed *);

    void slotArticleSelected(QListViewItem *);

    void fileOpen();
    void fileSaveAs();

    void slotFeedAdd();
    void slotFeedAddGroup();
    void slotFeedRemove();
    void slotFeedModify();
    void slotFeedCopy();
    void slotFetchCurrentFeed();
    void slotFetchAllFeeds();

    /**
     * This slot is called when user renames a feed in feeds tree.
     */
    void slotItemRenamed( QListViewItem * item ); //, const QString & text, int col

private:
    Feed *aKregatorPart::addFeed_Internal(QListViewItem *elt, QString title, QString xmlUrl, QString htmlUrl, QString description, bool isLiveJournal, QString ljUserName, Feed::LJAuthMode ljAuthMode, QString ljLogin, QString ljPassword, bool updateTitle);


    /**
     * A tree of all feeds (Columns).
     */
    FeedsTree *m_tree;

    /**
     * An internal collection of Feed items.
     */
    FeedsCollection m_feeds;

    /**
     * List of documents for currently selected feed.
     */
    ArticleList *m_articles;

    /**
     * Currently selected document renderer.
     */
    ArticleViewer *m_articleViewer;

    QSplitter *m_panner1, *m_panner2;
    QValueList<int> m_panner1Sep, m_panner2Sep;
};

class KInstance;
class KAboutData;

class aKregatorPartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    aKregatorPartFactory();
    virtual ~aKregatorPartFactory();
    virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
                                            QObject *parent, const char *name,
                                            const char *classname, const QStringList &args );
    static KInstance* instance();

private:
    static KInstance* s_instance;
    static KAboutData* s_about;
};

#endif // _AKREGATORPART_H_
