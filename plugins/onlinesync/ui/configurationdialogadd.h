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

#ifndef CONFIGURATIONDIALOGADD_H
#define CONFIGURATIONDIALOGADD_H

#include <KDialog>

#include "ui_configurationdialogadd.h"


namespace feedsync
{

class ConfigurationDialogAdd : public KDialog
{
    Q_OBJECT
    public:
        explicit ConfigurationDialogAdd( QWidget *parent=0 );
        ~ConfigurationDialogAdd();
        void load( const KConfigGroup &group );

    public Q_SLOTS:
        /* reimp */ void accept();

    private:
        Ui::ConfigurationDialogAdd ui;
        QString _baseconfigname;

    protected Q_SLOTS:
        void slotUpdateUI();
};
 
}

#endif // CONFIGURATIONDIALOGADD_H
