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

    QLabel *label = new QLabel(i18n("Akregator did not close correctly. Would you like to restore the previous session?"), this);
    label->setObjectName(QStringLiteral("restoresessionlabel"));
    vbox->addWidget(label);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    vbox->addLayout(buttonLayout);

    QPushButton *restoreSessionButton = new QPushButton(i18n("Restore Session"), this);
    restoreSessionButton->setObjectName(QStringLiteral("restoresessionbutton"));
    buttonLayout->addWidget(restoreSessionButton);

    QPushButton *dontRestoreSessionButton = new QPushButton(i18n("Do Not Restore Session"), this);
    dontRestoreSessionButton->setObjectName(QStringLiteral("dontrestoresessionbutton"));
    buttonLayout->addWidget(dontRestoreSessionButton);

    QPushButton *askMeLaterButton = new QPushButton(i18n("Ask me later"), this);
    askMeLaterButton->setObjectName(QStringLiteral("askmelaterbutton"));
    buttonLayout->addWidget(askMeLaterButton);


    setLayout(vbox);
}

CrashWidget::~CrashWidget()
{

}
