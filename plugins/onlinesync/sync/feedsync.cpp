/*
    This file is part of Akregator.

    Copyright (C) 2008 Didier Hoarau <did.hoarau@gmail.com>

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

#include "feedsync.h"
#include "treenode.h"
#include "feedlist.h"
#include "kernel.h"
#include "aggregator.h"
#include "googlereader.h"
#include "akregator.h"
#include "opml.h"

#include <QFile>
#include <QThread>
#include <QMessageBox>
#include <qdebug.h>
#include <kconfiggroup.h>

#include <QDate>
#include <QStandardPaths>

namespace feedsync {

FeedSync::FeedSync( QObject* p ) : QObject( p ) {
    qDebug();
    _aggrSend = 0;
    _aggrGet = 0;
}

FeedSync::~FeedSync() {
    qDebug();
}

feedsync::Aggregator * FeedSync::createAggregatorFactory(const KConfigGroup& configgroup) {
    qDebug() << configgroup.readEntry("Identifier");

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

    qDebug();

    // Sender
    QString m_account = QObject::sender()->property("ConfigGroup").toString();
    int m_synctype = QObject::sender()->property("SyncType").toInt();
    qDebug() << QObject::sender()->objectName() << m_account << m_synctype;
    KConfig config("akregator_feedsyncrc");
    KConfigGroup generalGroup( &config, m_account );

    // Init akregator
    // ### FIXME: this leaks
    Akregator * m_akr = new Akregator();
    m_akr->load();

    // Init 3rd party aggregator
    if (m_synctype==Get) {
        qDebug() << "Get feeds";
        _aggrGet = m_akr;
        _aggrSend = createAggregatorFactory(generalGroup);
    } else {
        qDebug() << "Send feeds";
        _aggrGet = createAggregatorFactory(generalGroup);
        _aggrSend = m_akr;
    }

    if (_aggrSend==0 || _aggrGet==0) {
        // TODO Notification
        qDebug() << "Error loading configuration";
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
    qDebug();
    _loadedAggrCount++;

    // All is loaded
    if (_loadedAggrCount==2) {

        // Calculate the add list
        tmp_addlist = _aggrGet->getSubscriptionList().compare( _aggrSend->getSubscriptionList(), SubscriptionList::Added );

        // Calculate the remove list
        SubscriptionList::RemovePolicy m_removepolicy = SubscriptionList::Nothing;
        // Calculate the Complete list
        SubscriptionList m_checkremove = _aggrGet->getSubscriptionList().compare(_aggrSend->getSubscriptionList(),SubscriptionList::Removed,SubscriptionList::Feed);
        if (!m_checkremove.isEmpty()) {
            // Check removal policy in the config
            KConfig config("akregator_feedsyncrc");
            KConfigGroup generalGroup( &config, "FeedSyncConfig" );
            // Feed
            if ( generalGroup.readEntry( "RemovalPolicy", QString() ) == "Feed" ) {
                qDebug() << "Policy: Remove feeds";
                m_removepolicy = SubscriptionList::Feed;
            // Category
            } else if (generalGroup.readEntry( "RemovalPolicy", QString() ) == "Category") {
                qDebug() << "Policy: Remove categories";
                m_removepolicy = SubscriptionList::Category;
            // Nothing
            } else if (generalGroup.readEntry( "RemovalPolicy", QString() ) == "Nothing") {
                qDebug() << "Policy: Remove nothing";
                m_removepolicy = SubscriptionList::Nothing;
            // Ask
            } else if (generalGroup.readEntry( "RemovalPolicy", QString() ) == "Ask") {
                QMessageBox msgBox;
                msgBox.setText(i18n("Some categories and feeds have been marked for removal. Do you want to delete them?"));
                msgBox.setIcon(QMessageBox::Information);
                /*QPushButton *noRemove  =*/ msgBox.addButton(i18n("Remove nothing"), QMessageBox::ActionRole);
                QPushButton *catRemove = msgBox.addButton(i18n("Remove only categories"), QMessageBox::ActionRole);
                QPushButton *feedRemove  = msgBox.addButton(i18n("Remove feeds"), QMessageBox::ActionRole);
                msgBox.exec();
                // Remove feed
                if (msgBox.clickedButton() == (QAbstractButton*) feedRemove) {
                    qDebug() << "Policy: Remove feeds";
                    m_removepolicy = SubscriptionList::Feed;
                // Remove only categories
                } else if (msgBox.clickedButton() == (QAbstractButton*) catRemove) {
                    qDebug() << "Policy: Remove categories";
                    m_removepolicy = SubscriptionList::Category;
                // Remove nothing
                } else {
                    qDebug() << "Policy: Remove nothing";
                    m_removepolicy = SubscriptionList::Nothing;
                }
            // Remove nothing
            } else {
                qDebug() << "Policy: Remove nothing";
                m_removepolicy = SubscriptionList::Nothing;
            }
        }
        if (m_removepolicy == SubscriptionList::Feed)
            tmp_removelist = m_checkremove;
        else
            tmp_removelist = _aggrGet->getSubscriptionList().compare( _aggrSend->getSubscriptionList() ,SubscriptionList::Removed, m_removepolicy );

        // Before: log
        log();

        // Now: Add
        _aggrGet->add(tmp_addlist);
    }
}

void FeedSync::slotAddDone() {
    qDebug();

    // Now: delete
   _aggrGet->remove(tmp_removelist);
}

void FeedSync::slotRemoveDone() {
    qDebug();
    delete _aggrSend;
    delete _aggrGet;
}

void FeedSync::error(const QString& msg) {
    qDebug();

    QMessageBox msgBox;
    if (msg.isEmpty()) {
        msgBox.setText(i18n("An error occurred, synchronization aborted."));
    } else {
        msgBox.setText(msg);
    }
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    delete _aggrSend;
    delete _aggrGet;
}

// Create a log

void FeedSync::log() {
    qDebug();

    QString logPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + '/' + "akregator" + "/onlinesync.log";

    QFile file(logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream out(&file);
    out << QDate::currentDate().toString(Qt::ISODate) << " " 
        << QTime::currentTime().toString(Qt::ISODate) << "\n";

    out << "To be added:" << "\n";
    for (int i=0; i<tmp_addlist.count(); i++) {
        out << "(+) xml:" << tmp_addlist.getRss(i) << " category:" << tmp_addlist.getCat(i) << "\n";
    }

    out << "To be removed:" << "\n";
    for (int i=0; i<tmp_removelist.count(); i++) {
        out << "(-) xml:" << tmp_removelist.getRss(i) << " category:" << tmp_removelist.getCat(i) << "\n";
    }
}

}
