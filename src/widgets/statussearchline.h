/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
