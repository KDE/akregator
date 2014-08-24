/*
    This file is part of Akregator.

    Copyright (C) 2006 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_BROWSERFRAME_P_H
#define AKREGATOR_BROWSERFRAME_P_H


// TODO: cleanup unneeded includes

#include "actionmanager.h"
#include "browserframe.h"
#include "openurlrequest.h"

#include <QAction>
#include <QGridLayout>
#include <QList>
#include <QPointer>
#include <QString>

#include <QAction>
#include <klibloader.h>
#include <kmimetypetrader.h>
#include <ktoolbarpopupaction.h>
#include <kurl.h>
#include <kxmlguiclient.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>

namespace Akregator {

class BrowserFrame::Private : public QObject
{
    Q_OBJECT

        BrowserFrame* const q;

    public:
        explicit Private(BrowserFrame* qq);
        ~Private();

        class HistoryEntry;
        class HistoryAction;

        QList<HistoryEntry> history;
        QList<HistoryEntry>::Iterator current;
        QPointer<KParts::ReadOnlyPart> part;
        QPointer<KParts::BrowserExtension> extension;
        QPointer<QGridLayout> layout;
        bool lockHistory;
        bool isLoading;

        QString mimetype;
        KService::Ptr service;

        void connectPart();
        void updateHistoryEntry();
        void appendHistoryEntry(const KUrl& url);
        void restoreHistoryEntry(const QList<HistoryEntry>::Iterator& entry);
        bool loadPartForMimetype(const QString& mimetype);

    public slots:
        void slotHistoryEntrySelected( const QList<BrowserFrame::Private::HistoryEntry>::Iterator& entry)
        {
            restoreHistoryEntry(entry);
        }
        void partDestroyed( QObject* obj );
};

class BrowserFrame::Private::HistoryEntry
{
    public:

        KUrl url;
        QString title;
        QByteArray buffer;
        int id;
        QString mimetype;
        QString strServiceName;
        QByteArray postData;
        QString postContentType;
        bool doPost;
        QString pageReferrer;

        HistoryEntry() : id(idCounter++) {}

        bool operator==(const HistoryEntry& other) const
        {
            return id == other.id;
        }

        bool operator!=(const HistoryEntry& other) const
        {
            return id != other.id;
        }

        static int idCounter;
};

class BrowserFrame::Private::HistoryAction : public QAction
{

    Q_OBJECT
    public:
        HistoryAction(QList<HistoryEntry>::Iterator entry,
                      QObject* q,
                      Private* priv);

        QList<HistoryEntry>::Iterator m_entry;

    public slots:

        void slotTriggered(bool);

    signals:

        void triggered(const QList<BrowserFrame::Private::HistoryEntry>::Iterator&);
};

} // namespace Akregator

#endif // AKREGATOR_BROWSERFRAME_P_H
