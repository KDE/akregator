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

#include "configurationdialog.h"
#include "configurationwidget.h"

#include <QDebug>

using namespace feedsync;

ConfigurationDialog::ConfigurationDialog(QWidget *parent) : KDialog(parent)
{
    qDebug();
    m_widget = new Ui::ConfigurationWidget(/*this*/);
    //setMainWidget( m_widget );
    setWindowTitle(i18n("Online Reader Configuration"));
}

void ConfigurationDialog::accept()
{
    qDebug();
    //m_widget->save();
    KDialog::accept();
}

