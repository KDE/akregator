#include "treenode.h"
#include "folder.h"
#include "feed.h"
#include "feedlist.h"
#include "kernel.h"

#include "akregator.h"
#include "subscriptionlist.h"

#include <kdebug.h>
#include <QDomDocument>
#include <QTimer>


namespace feedsync
{

Akregator::Akregator() 
{
    kDebug();
    _subscriptionList = new SubscriptionList();
}

Akregator::~Akregator() 
{
    kDebug();
    delete _subscriptionList;
}

SubscriptionList * Akregator::getSubscriptionList() const 
{
    kDebug();
    return _subscriptionList;
}

void Akregator::load() 
{
    using namespace Akregator;

    kDebug() << Kernel::self()->feedList()->toXML().toString().left(20);

    _xmlDoc = new QDomDocument("AkregatorOpml");
    _xmlDoc->setContent(Kernel::self()->feedList()->toXML().toString());

    QDomNodeList nodeList = _xmlDoc->elementsByTagName("outline");
    bool firstCat = true;
    QString m_cat;
    for (int i=0;i<nodeList.count();i++) {
        QDomNode node = nodeList.at(i);
        if (!node.attributes().namedItem("xmlUrl").isNull()) {
            _subscriptionList->add(node.attributes().namedItem("xmlUrl").nodeValue(),
                                   node.attributes().namedItem("title").nodeValue(),
                                   m_cat);
            firstCat = true;
        } else {
            if (firstCat) {
              m_cat = "";
              firstCat = false;
            } else {
              m_cat = m_cat + "/";
            }
            m_cat = m_cat + node.attributes().namedItem("text").nodeValue();
        }
    }

    // Send the signal
    QTimer::singleShot( 0, this, SLOT(sendSignalLoadDone()) );
}

void Akregator::sendSignalLoadDone() 
{
    emit loadDone();
}

void Akregator::add(SubscriptionList * list) 
{
    kDebug();

    // Get the feed list
    using namespace Akregator;
    for (int i=0; i<list->count(); i++) {
        kDebug() << list->getRss(i).left(20);

        FeedList * m_feedlist = Kernel::self()->feedList();
        std::auto_ptr<FeedList> new_feedlist( new FeedList( Kernel::self()->storage() ) );
        QList<TreeNode*> m_treenodelist = m_feedlist->asFlatList();

        // Find folder
        Folder * m_folder = 0;
        QString m_foldername = list->getCat(i,SubscriptionList::Simple);
        if (m_foldername=="") {
            m_folder = m_feedlist->rootNode();
        } else {
            for (int j=0;j<m_treenodelist.count();j++) {
                if (m_treenodelist.at(j)->parent()!=0) {
                    if ( m_treenodelist.at(j)->parent()->title() == m_foldername ) {
                        kDebug() << m_treenodelist.at(j)->parent()->title();
                        m_folder = m_treenodelist.at(j)->parent();
                        j=m_treenodelist.count();
                    }
                }
            }
            if (m_folder==0) {
                m_folder = new Folder(m_foldername);
                m_feedlist->rootNode()->appendChild(m_folder);
            }
        }

        // Get latest from in folder
        TreeNode* m_last = m_folder->childAt( m_folder->totalCount() );

        // Create new feed
        Feed * new_feed = new Feed( Kernel::self()->storage() );
        new_feed->setXmlUrl(list->getRss(i));
        new_feed->setTitle(list->getName(i));
        new_feedlist->rootNode()->appendChild(new_feed);

        // Add the feed
        m_feedlist->append(new_feedlist.get(), m_folder, m_last);
    }

    // Emit signal
    emit addDone();
}

void Akregator::update(SubscriptionList * list) 
{
    kDebug();

    // Emit signal
    emit updateDone();
}

void Akregator::remove(SubscriptionList * list) 
{
    kDebug();

    // Emit signal
    emit removeDone();
}

}
