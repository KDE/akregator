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

#include "statussearchline.h"
#include <KLocalizedString>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QDebug>

using namespace Akregator;
Q_DECLARE_METATYPE(Akregator::StatusSearchLine::Status)
StatusSearchLine::StatusSearchLine(QWidget *parent)
    : KLineEdit(parent),
      mDefaultStatus(AllArticles),
      mSearchLineStatusAction(Q_NULLPTR)
{
    initializeHash();
    initializeActions();
}

StatusSearchLine::~StatusSearchLine()
{

}

void StatusSearchLine::initializeHash()
{
    const QIcon iconAll = QIcon::fromTheme(QStringLiteral("system-run"));
    const QIcon iconNew = QIcon::fromTheme(QStringLiteral("mail-mark-unread-new"));
    const QIcon iconUnread = QIcon::fromTheme(QStringLiteral("mail-mark-unread"));
    const QIcon iconKeep = QIcon::fromTheme(QStringLiteral("mail-mark-important"));

    StatusInfo statusAll(i18n("All Articles"), iconAll);
    StatusInfo statusUnread(i18nc("Unread articles filter", "Unread"), iconUnread);
    StatusInfo statusNew(i18nc("New articles filter", "New"), iconNew);
    StatusInfo statusImportant(i18nc("Important articles filter", "Important"), iconKeep);

    mHashStatus.insert(AllArticles, statusAll);
    mHashStatus.insert(NewArticles, statusNew);
    mHashStatus.insert(UnreadArticles, statusUnread);
    mHashStatus.insert(ImportantArticles, statusImportant);
}

void StatusSearchLine::setStatus(StatusSearchLine::Status status)
{
    updateStatusIcon(status);
}

void StatusSearchLine::initializeActions()
{
    mSearchLineStatusAction = addAction(mHashStatus.value(AllArticles).mIcon, QLineEdit::LeadingPosition);
    mSearchLineStatusAction->setToolTip(mHashStatus.value(AllArticles).mText);
    connect(mSearchLineStatusAction, &QAction::triggered, this, &StatusSearchLine::showMenu);
}

void StatusSearchLine::showMenu()
{
    QMenu p(this);
    QActionGroup *grp = new QActionGroup(this);
    grp->setExclusive(true);
    for (int i = AllArticles; i <= ImportantArticles; ++i) {
        StatusSearchLine::Status status = static_cast<StatusSearchLine::Status>(i);
        QAction *act = new QAction(mHashStatus.value(status).mIcon, mHashStatus.value(status).mText, this);
        act->setCheckable(true);
        act->setChecked(mDefaultStatus == status);
        act->setData(QVariant::fromValue(status));
        grp->addAction(act);
        p.addAction(act);
        if (i == AllArticles) {
            p.addSeparator();
        }
    }
    QAction *a = p.exec(mapToGlobal(QPoint(0, height())));
    if (a) {
        const StatusSearchLine::Status newStatus = a->data().value<StatusSearchLine::Status>();
        updateStatusIcon(newStatus);
    }
}

void StatusSearchLine::updateStatusIcon(StatusSearchLine::Status status)
{
    if (mDefaultStatus != status) {
        mDefaultStatus = status;
        mSearchLineStatusAction->setIcon(mHashStatus[status].mIcon);
        mSearchLineStatusAction->setToolTip(mHashStatus[status].mText);
        Q_EMIT statusChanged(mDefaultStatus);
    }
}

StatusSearchLine::Status StatusSearchLine::status() const
{
    return mDefaultStatus;
}
