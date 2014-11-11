/*
    This file is part of Akregator.

    Copyright (C) 2004 Stanislav Karchebny <Stanislav.Karchebny@kdemail.net>

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

#ifndef AKREGATOR_ADDFEEDDIALOG_H
#define AKREGATOR_ADDFEEDDIALOG_H

#include "ui_addfeedwidgetbase.h"

#include <qdialog.h>

#include <QWidget>
class QPushButton;
namespace Akregator
{

class Feed;

class AddFeedWidget : public QWidget, public Ui::AddFeedWidgetBase
{
    Q_OBJECT
public:
    explicit AddFeedWidget(QWidget *parent = 0, const char *name = 0);
    ~AddFeedWidget();
};

class AddFeedDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddFeedDialog(QWidget *parent = 0, const char *name = 0);
    ~AddFeedDialog();

    QSize sizeHint() const;

    void setUrl(const QString &t);
    Feed *feed();

public slots:
    void accept();

    void fetchCompleted(Akregator::Feed *);
    void fetchDiscovery(Akregator::Feed *);
    void fetchError(Akregator::Feed *);

private slots:
    void textChanged(const QString &);

private:
    AddFeedWidget *widget;
    QString feedUrl;
    Feed *m_feed;
    QPushButton *mOkButton;
};

} // namespace Akregator

#endif // AKREGATOR_ADDFEEDDIALOG_H
