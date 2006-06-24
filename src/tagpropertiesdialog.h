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

#ifndef AKREGATOR_TAGPROPERTIESDIALOG_H
#define AKREGATOR_TAGPROPERTIESDIALOG_H

#include <kdialog.h>

namespace Akregator {

class Tag;

class TagPropertiesDialog : public KDialog
{
    Q_OBJECT

    public:

        TagPropertiesDialog(QWidget *parent=0, const char *name=0);
        virtual ~TagPropertiesDialog();

        void setTag(const Tag& tag);

    protected slots:

        virtual void slotOk();
        virtual void slotApply();
        virtual void slotTextChanged(const QString& text);

    private:
        class TagPropertiesDialogPrivate;
        TagPropertiesDialogPrivate* d;
};

} // namespace Akregator

#endif // AKREGATOR_TAGPROPERTIESDIALOG_H
