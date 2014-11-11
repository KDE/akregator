/*
    This file is part of Akregator.

    Copyright (C) 2008 Frank Osterfeld <osterfeld@kde.org>

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

#ifndef AKREGATOR_CREATEFEEDCOMMAND_H
#define AKREGATOR_CREATEFEEDCOMMAND_H

#include "command.h"

namespace Akregator
{

class Folder;
class MainWidget;
class SubscriptionListView;
class TreeNode;

class CreateFeedCommand : public Command
{
    Q_OBJECT
public:
    explicit CreateFeedCommand(MainWidget *parent = 0);
    ~CreateFeedCommand();

    void setSubscriptionListView(SubscriptionListView *view);
    void setRootFolder(Folder *rootFolder);
    void setUrl(const QString &url);
    void setPosition(Folder *parent, TreeNode *after);
    void setAutoExecute(bool autoexec);

private:
    void doStart();
    void doAbort();

private:
    class Private;
    Private *const d;
    Q_PRIVATE_SLOT(d, void doCreate())
};

}

#endif // AKREGATOR_CREATEFEEDCOMMAND_H
