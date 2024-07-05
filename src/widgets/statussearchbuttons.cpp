/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "statussearchbuttons.h"
#include <KLocalizedString>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QToolButton>
using namespace Akregator;
StatusSearchButtons::StatusSearchButtons(QWidget *parent)
    : QWidget{parent}
    , mButtonGroup(new QButtonGroup(this))
{
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setObjectName(QStringLiteral("mainLayout"));
    mainLayout->setContentsMargins({});
    mainLayout->setSpacing(0);

    mButtonGroup->setObjectName(QStringLiteral("mButtonGroup"));
    mButtonGroup->setExclusive(true);

    // All
    auto toolButton = new QToolButton(this);
    toolButton->setCheckable(true);
    toolButton->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
    toolButton->setToolTip(i18n("All Articles"));
    toolButton->setAutoRaise(true);
    mainLayout->addWidget(toolButton, 0, Qt::AlignTop);
    mButtonGroup->addButton(toolButton, StatusSearchLine::Status::AllArticles);

    // NewArticles
    toolButton = new QToolButton(this);
    toolButton->setCheckable(true);
    toolButton->setAutoRaise(true);
    toolButton->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-unread-new")));
    toolButton->setToolTip(i18nc("New articles filter", "New"));
    mainLayout->addWidget(toolButton, 0, Qt::AlignTop);
    mButtonGroup->addButton(toolButton, StatusSearchLine::Status::NewArticles);

    // Unread
    toolButton = new QToolButton(this);
    toolButton->setCheckable(true);
    toolButton->setAutoRaise(true);
    toolButton->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-unread")));
    toolButton->setToolTip(i18nc("Unread articles filter", "Unread"));
    mainLayout->addWidget(toolButton, 0, Qt::AlignTop);
    mButtonGroup->addButton(toolButton, StatusSearchLine::Status::UnreadArticles);

    // Read
    toolButton = new QToolButton(this);
    toolButton->setCheckable(true);
    toolButton->setAutoRaise(true);
    toolButton->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-read")));
    toolButton->setToolTip(i18nc("Read articles filter", "Read"));
    mainLayout->addWidget(toolButton, 0, Qt::AlignTop);
    mButtonGroup->addButton(toolButton, StatusSearchLine::Status::ReadArticles);

    // Important
    toolButton = new QToolButton(this);
    toolButton->setCheckable(true);
    toolButton->setAutoRaise(true);
    toolButton->setIcon(QIcon::fromTheme(QStringLiteral("mail-mark-important")));
    toolButton->setToolTip(i18nc("Important articles filter", "Important"));
    mainLayout->addWidget(toolButton, 0, Qt::AlignTop);
    mButtonGroup->addButton(toolButton, StatusSearchLine::Status::ImportantArticles);
    connect(mButtonGroup, &QButtonGroup::idClicked, this, [this](int index) {
        Q_EMIT statusChanged(static_cast<StatusSearchLine::Status>(index));
    });
}

StatusSearchButtons::~StatusSearchButtons() = default;

void StatusSearchButtons::setStatus(StatusSearchLine::Status status)
{
    auto b = mButtonGroup->button(status);
    if (b) {
        b->setChecked(true);
    }
}

StatusSearchLine::Status StatusSearchButtons::status() const
{
    return static_cast<StatusSearchLine::Status>(mButtonGroup->checkedId());
}

#include "moc_statussearchbuttons.cpp"
