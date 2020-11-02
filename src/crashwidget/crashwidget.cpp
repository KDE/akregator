/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "crashwidget.h"
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QPushButton>
#include <QLabel>

using namespace Akregator;

CrashWidget::CrashWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *vbox = new QVBoxLayout(this);

    auto *labelLayout = new QHBoxLayout;
    QLabel *label = new QLabel(i18n("Akregator did not close correctly. Would you like to restore the previous session?"), this);
    label->setObjectName(QStringLiteral("restoresessionlabel"));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QFont font = label->font();
    font.setBold(true);
    font.setPixelSize(20);
    label->setFont(font);
    vbox->addLayout(labelLayout);
    labelLayout->addStretch(0);
    labelLayout->addWidget(label);
    labelLayout->addStretch(0);

    auto *buttonLayout = new QHBoxLayout;
    vbox->addLayout(buttonLayout);
    buttonLayout->addStretch(0);

    QPushButton *restoreSessionButton = new QPushButton(QIcon::fromTheme(QStringLiteral("window-new")), i18n("Restore Session"), this);
    restoreSessionButton->setObjectName(QStringLiteral("restoresessionbutton"));
    restoreSessionButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonLayout->addWidget(restoreSessionButton);
    connect(restoreSessionButton, &QPushButton::clicked, this, &CrashWidget::slotRestoreSession);

    QPushButton *dontRestoreSessionButton = new QPushButton(QIcon::fromTheme(QStringLiteral("dialog-close")), i18n("Do Not Restore Session"), this);
    dontRestoreSessionButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    dontRestoreSessionButton->setObjectName(QStringLiteral("dontrestoresessionbutton"));
    buttonLayout->addWidget(dontRestoreSessionButton);
    connect(dontRestoreSessionButton, &QPushButton::clicked, this, &CrashWidget::slotDontRestoreSession);

    QPushButton *askMeLaterButton = new QPushButton(QIcon::fromTheme(QStringLiteral("chronometer")), i18n("Ask me later"), this);
    askMeLaterButton->setObjectName(QStringLiteral("askmelaterbutton"));
    buttonLayout->addWidget(askMeLaterButton);
    connect(askMeLaterButton, &QPushButton::clicked, this, &CrashWidget::slotAskMeLater);
    askMeLaterButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    buttonLayout->addStretch(0);
}

CrashWidget::~CrashWidget()
{
}

void CrashWidget::slotRestoreSession()
{
    Q_EMIT restoreSession(RestoreSession);
}

void CrashWidget::slotDontRestoreSession()
{
    Q_EMIT restoreSession(NotRestoreSession);
}

void CrashWidget::slotAskMeLater()
{
    Q_EMIT restoreSession(AskMeLater);
}
