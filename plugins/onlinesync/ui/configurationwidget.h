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

#ifndef CONFIGURATIONWIDGET_H_
#define CONFIGURATIONWIDGET_H_

#include "ui_configurationwidget.h"

#include <QWidget>

namespace feedsync
{

class ConfigurationWidget : public QWidget
{
    Q_OBJECT
    public:
        // Constructor if the dialog must be created
        explicit ConfigurationWidget( QWidget *parent=0 );
        ~ConfigurationWidget();

        void load();
        void save();

    private:
        Ui::ConfigurationWidget ui;

    private Q_SLOTS:
        void slotButtonAddClicked();
        void slotButtonUpdateClicked();
        void slotButtonRemoveClicked();
        void refresh();
};
 
}

#endif /*CONFIGURATIONWIDGET_H_*/
