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

#ifndef AKREGATOR_ONLINESYNC_PLUGIN_H
#define AKREGATOR_ONLINESYNC_PLUGIN_H

#include "plugin.h"

#include <kparts/plugin.h>

#include <QPointer>

class KActionMenu;

namespace feedsync
{
class FeedSync;
}

namespace Akregator
{

class OnlineSyncPlugin;

class OnlineSyncPluginIface : public Plugin
{
    Q_OBJECT
public:
    explicit OnlineSyncPluginIface(QObject *parent = 0, const QList<QVariant> &args = QList<QVariant>());
    ~OnlineSyncPluginIface();

    /* reimp */ void insertGuiClients(KXMLGUIClient *parent);
    /* reimp */ void removeGuiClients(KXMLGUIClient *parent);

    /* reimp */ void doInitialize() {}
private:
    QPointer<OnlineSyncPlugin> m_impl;
};

class OnlineSyncPlugin : public KParts::Plugin
{
    Q_OBJECT

public:
    explicit OnlineSyncPlugin(QObject *parent = 0, const QList<QVariant> &args = QList<QVariant>());
    ~OnlineSyncPlugin();

private:
    void updateActions();

private Q_SLOTS:
    void doSynchronize();
    /** open the feed synchronization management */
    void slotFeedSyncManage();
    void slotFeedSyncManageDone();

private:
    feedsync::FeedSync *m_syncTool;
    KActionMenu *m_feedSyncMenu;
    QList<KAction *> m_feedSyncActions;
};

} // namespace Akregator

#endif // AKREGATOR_ONLINESYNC_PLUGIN_H
