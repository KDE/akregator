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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

//#include "menuitems.h"
#include "tag.h"
#include "tagaction.h"

#include <kactioncollection.h>
#include <kapplication.h>
#include <kauthorized.h>
#include <kdebug.h>
#include <kmenu.h>

#include <QMap>

namespace Akregator {

class TagAction::TagActionPrivate
{
    public:
    Tag tag;
    //QMap<int, QPopupMenu*> idToPopup;
    //QMap<QPopupMenu*, int> popupToId;
};
 
TagAction::TagAction(const Tag& tag, const QObject* receiver, const char* slot, KActionCollection* parent)
//KAction (const QString &text, const KShortcut &cut, const QObject *receiver, const char *slot, QObject *parent, const char *name=0)
       : KToggleAction(tag.name(), KShortcut(), 0, 0, parent), d(new TagActionPrivate)
{
     d->tag = tag;
     connect(this, SIGNAL(toggled(const Tag&, bool)), receiver, slot);
     connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
}

TagAction::~TagAction()
{
    delete d;
    d = 0;
}

Tag TagAction::tag() const
{
    return d->tag;
}

/*
void TagAction::unplug(QWidget* widget)
{
    KToggleAction::unplug(widget);

    QPopupMenu* popup = ::qt_cast<QPopupMenu *>(widget);
    if (popup)
    {
        d->idToPopup.remove(d->popupToId[popup]);
        d->popupToId.remove(popup);
    }
}*/

/*
int TagAction::plug(QWidget* widget, int index)
{
    QPopupMenu* popup = ::qt_cast<QPopupMenu *>( widget );
    if (!popup)
    {
        kWarning() << "Can not plug KToggleAction in " << widget->className() << endl;
        return -1;
    }
    if (!KAuthorized::authorizeKAction(name()))
        return -1;
    
   TagMenuItem* item = new TagMenuItem(d->tag);
    int id = popup->insertItem(TagMenuItem::checkBoxIconSet(isChecked(), popup->colorGroup()), item, -1, index);
   
    
    popup->connectItem (id, this, SLOT(slotActivated()));

    d->popupToId[popup] = id;
    d->idToPopup[id] = popup;

    if ( id == -1 )
        return id;
    
    return id;   
}
*/
void TagAction::slotToggled(bool enabled)
{
    emit toggled(d->tag, enabled);
}

} // namespace Akregator

#include "tagaction.moc"
