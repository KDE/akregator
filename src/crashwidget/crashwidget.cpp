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

#include "crashwidget.h"
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QPushButton>
#include <QLabel>

using namespace Akregator;

CrashWidget::CrashWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *labelLayout = new QHBoxLayout;
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

    QHBoxLayout *buttonLayout = new QHBoxLayout;
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
    setLayout(vbox);
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
    Q_EMIT restoreSession(RestoreSession);
}

void CrashWidget::slotAskMeLater()
{
    Q_EMIT restoreSession(AskMeLater);
}
