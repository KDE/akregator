/*
    This file is part of Akregator.

    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>

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

#ifndef AKREGATOR_SHAREMICROBLOGPLUGIN_H
#define AKREGATOR_SHAREMICROBLOGPLUGIN_H

#include "plugin.h"
#include "article.h"
#include <kparts/plugin.h>
#include <KPluginFactory>
#include <Plasma/DataEngine>

class KActionMenu;

namespace Plasma
{
class ServiceJob;
}

class SharePlugin : public KParts::Plugin
{
    Q_OBJECT

public:
    explicit SharePlugin(QObject *parent = 0, const QList<QVariant> &args = QList<QVariant>());
    ~SharePlugin();

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);

protected Q_SLOTS:
    void articlesSelected(const QList<Akregator::Article> &articles);
    void slotServiceFinished(Plasma::ServiceJob *job);
    void shareArticles();

private:
    void refreshConfig();

    KActionMenu *m_shareMenu;
    KActionMenu *m_sharePopupMenu;
    QString m_username;
    QString m_serviceUrl;
    QList<Akregator::Article> m_articles;
    Plasma::DataEngine *m_engine;
    Plasma::Service *m_service;
};

class SharePluginIface : public Akregator::Plugin
{
    Q_OBJECT

public:
    SharePluginIface(QObject *parent, const QVariantList &params);
    ~SharePluginIface();

    /* reimp */ void doInitialize();
    /* reimp */ void insertGuiClients(KXMLGUIClient *parent);
    /* reimp */ void removeGuiClients(KXMLGUIClient *parent);

private:
    QPointer<SharePlugin> m_impl;
};

#endif // AKREGATOR_SHAREPLUGIN_H
