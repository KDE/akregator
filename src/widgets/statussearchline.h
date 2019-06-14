/*
   Copyright (C) 2016-2019 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef STATUSSEARCHLINE_H
#define STATUSSEARCHLINE_H

#include <KLineEdit>
#include <QHash>
#include <QIcon>
class QAction;
namespace Akregator {
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

    explicit StatusSearchLine(QWidget *parent = nullptr);
    ~StatusSearchLine();

    void setStatus(StatusSearchLine::Status status);

    Q_REQUIRED_RESULT Status status() const;

Q_SIGNALS:
    void statusChanged(Akregator::StatusSearchLine::Status status);
    void forceLostFocus();

protected:
    void keyPressEvent(QKeyEvent *e) override;

private Q_SLOTS:
    void showMenu();

private:
    struct StatusInfo {
        StatusInfo()
        {
        }

        StatusInfo(const QString &text, const QIcon &icon)
            : mText(text)
            , mIcon(icon)
        {
        }

        QString mText;
        QIcon mIcon;
    };

    void initializeHash();
    void initializeActions();
    void updateStatusIcon(StatusSearchLine::Status status);
    Status mDefaultStatus = AllArticles;
    QHash<StatusSearchLine::Status, StatusInfo> mHashStatus;
    QAction *mSearchLineStatusAction = nullptr;
};
}
#endif // STATUSSEARCHLINE_H
