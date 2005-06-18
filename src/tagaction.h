/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef AKREGATOR_TAGACTION_H
#define AKREGATOR_TAGACTION_H

#include <kaction.h>

class QString;

namespace Akregator {

class Tag;

/** a KAction for assigning and removing tags. Displays the tag name and emits the tag when activated */
class TagAction : public KAction
{
    Q_OBJECT

    public:

        TagAction(const Tag& tag, const QObject *receiver, const char *slot, QObject *parent=0);
        virtual ~TagAction();

    signals:

        void activated(const Tag&);

    protected: 

        virtual void slotActivated();

    private:

        class TagActionPrivate;
        TagActionPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_TAGACTION_H
