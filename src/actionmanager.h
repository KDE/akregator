/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#ifndef AKREGATOR_ACTIONMANAGER_H
#define AKREGATOR_ACTIONMANAGER_H

#include <qobject.h>

class KAction;

class QWidget;

namespace Akregator {

/**
 * interface for accessing actions, popup menus etc. from
 * widgets.
 * (Extracted from the implementation to avoid dependencies
 *  between widgets and  Akregator::Part).
 */
class ActionManager : public QObject
{
    public:

        static ActionManager* getInstance();
        static void setInstance(ActionManager* manager);

        ActionManager(QObject* parent=0, const char* name=0);
        virtual ~ActionManager();

        virtual KAction* action(const char* name) = 0;
        virtual QWidget* container(const char* name) = 0;
        
    private:

        static ActionManager* m_self;

        class ActionManagerPrivate;
        ActionManagerPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_ACTIONMANAGER_H
