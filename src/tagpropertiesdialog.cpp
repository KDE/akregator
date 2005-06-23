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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <klocale.h>

#include <qlineedit.h>

#include "tag.h"
#include "tagpropertiesdialog.h"
#include "tagpropertieswidgetbase.h"

namespace Akregator {

class TagPropertiesDialog::TagPropertiesDialogPrivate
{
    public:
    Tag tag;
    TagPropertiesWidgetBase* widget;
};

TagPropertiesDialog::TagPropertiesDialog(QWidget *parent, const char *name) : KDialogBase(KDialogBase::Swallow, Qt::WStyle_DialogBorder, parent, name, true, i18n("Tag Properties"), KDialogBase::Ok|KDialogBase::Cancel), d(new TagPropertiesDialogPrivate)
{
    d->widget = new TagPropertiesWidgetBase(this);
    setMainWidget(d->widget);
    d->widget->le_title->setFocus();
}

TagPropertiesDialog::~TagPropertiesDialog()
{
    delete d;
    d = 0;
}

void TagPropertiesDialog::setTag(const Tag& tag)
{
    d->tag = tag;
    d->widget->le_title->setText(tag.name());
}

void TagPropertiesDialog::slotOk()
{
    d->tag.setName(d->widget->le_title->text());
    KDialogBase::slotOk();
}

}  // namespace Akregator

#include "tagpropertiesdialog.moc"
