/*
    This file is part of Akregator.

    Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
#include "pageviewer.h"

#include <QAction>
#include <QGridLayout>
#include <QList>
#include <QPoint>
#include <QString>
#include <QWidget>

#include <kaction.h>
#include <kdebug.h>
#include <klibloader.h>
#include <kmenu.h>
#include <kmimetypetrader.h>
#include <ktoolbarpopupaction.h>
#include <kurl.h>
#include <kxmlguiclient.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>

namespace Akregator {

class BrowserFrame::BrowserFramePrivate : public QObject
{
    Q_OBJECT

    public:
        BrowserFramePrivate(BrowserFrame* p) : QObject(), parent(p) {}

        class HistoryEntry;
        class HistoryAction;
        
        QList<HistoryEntry> history;
        QList<HistoryEntry>::Iterator current;
        KParts::ReadOnlyPart* part;
        KParts::BrowserExtension* extension;
        QGridLayout* layout;
        bool lockHistory;
        BrowserFrame* parent;

        QString mimetype;
        KService::Ptr service;

        void connectPart();
        void updateHistoryEntry();
        void appendHistoryEntry();
        void restoreHistoryEntry(QList<HistoryEntry>::Iterator entry);
        bool loadPartForMimetype(const QString& mimetype);
    
        QString debugInfo() const;
        
    public slots:
        void slotHistoryEntrySelected(QList<BrowserFrame::BrowserFramePrivate::HistoryEntry>::Iterator entry)
        {
            restoreHistoryEntry(entry);
        }
};

class BrowserFrame::BrowserFramePrivate::HistoryEntry
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

        bool operator==(const HistoryEntry& other)
        {
            return id == other.id;
        }

        bool operator!=(const HistoryEntry& other)
        {
            return id != other.id;
        }

        static int idCounter;
};
class BrowserFrame::BrowserFramePrivate::HistoryAction : public QAction
{
    
    Q_OBJECT            
    public:
        HistoryAction(QList<HistoryEntry>::Iterator entry, 
                      QObject* parent,
                      BrowserFramePrivate* priv);
        
        QList<HistoryEntry>::Iterator m_entry;
    
    public slots:
        
        void slotTriggered(bool);
            
    signals:
        
        void triggered(QList<BrowserFrame::BrowserFramePrivate::HistoryEntry>::Iterator);
};

} // namespace Akregator

#endif // AKREGATOR_BROWSERFRAME_P_H
