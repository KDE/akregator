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

#include "tagaction.h"

namespace Akregator {


TagAction::TagAction(const QString& tagName, const QObject *receiver, const char *slot, QObject *parent) 
//KAction (const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, QObject *parent, const char *name=0)
       : KAction(tagName, KShortcut(), 0, 0, parent)
       , m_tagName(tagName)
{
    connect(this, SIGNAL(activated(const QString&)), receiver, slot);
}

TagAction::~TagAction()
{}

void TagAction::slotActivated()
{
    emit activated(m_tagName);
    KAction::slotActivated();
}

} // namespace Akregator

#include "tagaction.moc"
