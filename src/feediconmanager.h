/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_FEEDICONMANAGER_H
#define AKREGATOR_FEEDICONMANAGER_H

#include "akregator_export.h"
#include <QObject>

class KUrl;

class QIcon;
class QString;

namespace Akregator
{

class AKREGATOR_EXPORT FaviconListener
{
public:
    virtual ~FaviconListener();

    virtual void setFavicon(const QIcon &icon) = 0;
};

class AKREGATOR_EXPORT FeedIconManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.akregator.feediconmanager")

public:
    static FeedIconManager *self();

    ~FeedIconManager();

    void addListener(const KUrl &url, FaviconListener *listener);
    void removeListener(FaviconListener *listener);

private Q_SLOTS:
    Q_SCRIPTABLE void slotIconChanged(bool, const QString &, const QString &);

private:
    FeedIconManager();

    class Private;
    friend class ::Akregator::FeedIconManager::Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void loadIcon(QString))
};

} // namespace Akregator

#endif // AKREGATOR_FEEDICONMANAGER_H
