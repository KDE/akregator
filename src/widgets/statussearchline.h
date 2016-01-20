/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef STATUSSEARCHLINE_H
#define STATUSSEARCHLINE_H

#include <KLineEdit>
#include <QHash>
class QAction;
namespace Akregator
{
class StatusSearchLine : public KLineEdit
{
    Q_OBJECT
public:
    enum Status {
        AllArticles = 0,
        NewArticles,
        UnreadArticles,
        ImportantArticles
    };

    explicit StatusSearchLine(QWidget *parent = Q_NULLPTR);
    ~StatusSearchLine();

    void setStatus(StatusSearchLine::Status status);

private Q_SLOTS:
    void showMenu();

private:
    void initializeHash();
    void initializeActions();
    void updateStatusIcon(StatusSearchLine::Status status);
    QHash<StatusSearchLine::Status, QIcon> mHashStatus;
    QAction *mSearchLineStatusAction;
};
}
#endif // STATUSSEARCHLINE_H
