#include <KAboutData>
#include <KCmdLineArgs>
#include <QTimer>
#include <QFile>
#include <QThread>
#include <QMessageBox>
#include <QDomElement>
#include <QDomDocument>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <QDate>

#include "treenode.h"
#include "feedlist.h"
#include "kernel.h"
#include "aggregator.h"
#include "feedsync.h"
#include "googlereader.h"
#include "akregator.h"
#include "opml.h"

namespace feedsync {

FeedSync::FeedSync() {
    kDebug();
    _aggrSend = 0;
    _aggrGet = 0;
    tmp_removelist = 0;
    tmp_addlist = 0;
}

FeedSync::~FeedSync() {
    kDebug();
}

feedsync::Aggregator * FeedSync::createAggregatorFactory(KConfigGroup configgroup) {
    kDebug() << configgroup.readEntry("Identifier");

    Aggregator * m_agg;

    // If Google Reader
    if ( configgroup.readEntry("AggregatorType")== "GoogleReader") {
        m_agg = new GoogleReader(configgroup);
        m_agg->load();
    // If Opml
    } else if ( configgroup.readEntry("AggregatorType")== "Opml") {
        m_agg = new Opml(configgroup);
        m_agg->load();
    // Wrong type
    } else {
        return 0;
    }

    return m_agg;
}

void FeedSync::sync() {

    kDebug();

    // Sender
    QString m_account = QObject::sender()->property("ConfigGroup").toString();
    int m_synctype = QObject::sender()->property("SyncType").toInt();
    kDebug() << QObject::sender()->objectName() << m_account << m_synctype;
    KConfig config("akregator_feedsyncrc");
    KConfigGroup generalGroup( &config, m_account );

    // Init akregator
    Akregator * m_akr = new Akregator();
    m_akr->load();

    // Init 3rd party aggregator
    if (m_synctype==Get) {
        kDebug() << "Get feeds";
        _aggrGet = m_akr;
        _aggrSend = createAggregatorFactory(generalGroup);
    } else {
        kDebug() << "Send feeds";
        _aggrGet = createAggregatorFactory(generalGroup);
        _aggrSend = m_akr;
    }

    if (_aggrSend==0 or _aggrGet==0) {
        // TODO Notification
        kDebug() << "Error loading configuration";
    }


    // TODO Handle error cases

    // TODO Handle failures
    _loadedAggrCount=0;
    connect(_aggrGet, SIGNAL(error(QString)), this, SLOT(error(QString)));
    connect(_aggrSend, SIGNAL(error(QString)), this, SLOT(error(QString)));
    connect(_aggrSend, SIGNAL(loadDone()), this, SLOT(slotLoadDone()));
    connect(_aggrGet, SIGNAL(loadDone()), this, SLOT(slotLoadDone()));
    connect(_aggrGet, SIGNAL(addDone()), this, SLOT(slotAddDone()));
    connect(_aggrGet, SIGNAL(removeDone()), this, SLOT(slotRemoveDone()));
}

// Slots

void FeedSync::slotLoadDone() {
    kDebug();
    _loadedAggrCount++;

    // All is loaded
    if (_loadedAggrCount==2) {

        // Calculate the add list
        tmp_addlist = _aggrGet->getSubscriptionList()->compare( _aggrSend->getSubscriptionList(), SubscriptionList::Added );

        // Calculate the remove list
        SubscriptionList::RemovePolicy m_removepolicy = SubscriptionList::Nothing;
        // Calculate the Complete list
        SubscriptionList * m_checkremove = _aggrGet->getSubscriptionList()->compare(_aggrSend->getSubscriptionList(),SubscriptionList::Removed,SubscriptionList::Feed);
        if (m_checkremove->count()>0) {
            // Check removal policy in the config
            KConfig config("akregator_feedsyncrc");
            KConfigGroup generalGroup( &config, "FeedSyncConfig" );
            // Feed
            if ( generalGroup.readEntry( "RemovalPolicy", QString() ) == "Feed" ) {
                kDebug() << "Policy: Remove feeds";
                m_removepolicy = SubscriptionList::Feed;
            // Category
            } else if (generalGroup.readEntry( "RemovalPolicy", QString() ) == "Category") {
                kDebug() << "Policy: Remove categories";
                m_removepolicy = SubscriptionList::Category;
            // Nothing
            } else if (generalGroup.readEntry( "RemovalPolicy", QString() ) == "Nothing") {
                kDebug() << "Policy: Remove nothing";
                m_removepolicy = SubscriptionList::Nothing;
            // Ask
            } else if (generalGroup.readEntry( "RemovalPolicy", QString() ) == "Ask") {
                QMessageBox msgBox;
                msgBox.setText(i18n("Some categories and feeds have been marked for removal. Do you want to delete them?"));
                msgBox.setIcon(QMessageBox::Information);
                QPushButton *noRemove  = msgBox.addButton(i18n("Remove nothing"), QMessageBox::ActionRole);
                QPushButton *catRemove = msgBox.addButton(i18n("Remove only categories"), QMessageBox::ActionRole);
                QPushButton *feedRemove  = msgBox.addButton(i18n("Remove feeds"), QMessageBox::ActionRole);
                msgBox.exec();
                // Remove feed
                if (msgBox.clickedButton() == (QAbstractButton*) feedRemove) {
                    kDebug() << "Policy: Remove feeds";
                    m_removepolicy = SubscriptionList::Feed;
                // Remove only categories
                } else if (msgBox.clickedButton() == (QAbstractButton*) catRemove) {
                    kDebug() << "Policy: Remove categories";
                    m_removepolicy = SubscriptionList::Category;
                // Remove nothing
                } else {
                    kDebug() << "Policy: Remove nothing";
                    m_removepolicy = SubscriptionList::Nothing;
                }
            // Remove nothing
            } else {
                kDebug() << "Policy: Remove nothing";
                m_removepolicy = SubscriptionList::Nothing;
            }
        }
        if (m_removepolicy == SubscriptionList::Feed) {
            tmp_removelist = m_checkremove;
        } else {
            delete m_checkremove;
            tmp_removelist = _aggrGet->getSubscriptionList()->compare( _aggrSend->getSubscriptionList() ,SubscriptionList::Removed, m_removepolicy );
        }

        // Before: log
        log();

        // Now: Add
        _aggrGet->add(tmp_addlist);
    }
}

void FeedSync::slotAddDone() {
    kDebug();

    // Now: delete
   _aggrGet->remove(tmp_removelist);
}

void FeedSync::slotRemoveDone() {
    kDebug();
    if (_aggrSend!=0) {
        delete _aggrSend;
    }
    if (_aggrGet!=0) {
        delete _aggrGet;
    }
    if (tmp_removelist!=0) {
        delete tmp_removelist;
    }
    if (tmp_addlist!=0) {
        delete tmp_addlist;
    }
}

void FeedSync::error(const QString& msg) {
    kDebug();

    QMessageBox msgBox;
    if (msg.isEmpty()) {
        msgBox.setText(i18n("An error occurred, synchronization aborted."));
    } else {
        msgBox.setText(msg);
    }
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();

    if (_aggrSend!=0) {
        delete _aggrSend;
    }
    if (_aggrGet!=0) {
        delete _aggrGet;
    }
    if (tmp_removelist!=0) {
        delete tmp_removelist;
    }
    if (tmp_addlist!=0) {
        delete tmp_addlist;
    }
}

// Create a log

void FeedSync::log() {
    kDebug();

    QString logPath = KGlobal::dirs()->saveLocation("data", "akregator") + "/onlinesync.log";

    QFile file(logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream out(&file);
    out << QDate::currentDate().toString(Qt::ISODate) << " " 
        << QTime::currentTime().toString(Qt::ISODate) << "\n";

    out << "To be added:" << "\n";
    for (int i=0; i<tmp_addlist->count(); i++) {
        out << "(+) xml:" << tmp_addlist->getRss(i) << " category:" << tmp_addlist->getCat(i) << "\n";
    }

    out << "To be removed:" << "\n";
    for (int i=0; i<tmp_removelist->count(); i++) {
        out << "(-) xml:" << tmp_removelist->getRss(i) << " category:" << tmp_removelist->getCat(i) << "\n";
    }
}

}
