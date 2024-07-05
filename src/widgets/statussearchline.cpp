/*
   SPDX-FileCopyrightText: 2016-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "statussearchline.h"
#include <QKeyEvent>

using namespace Akregator;
Q_DECLARE_METATYPE(Akregator::StatusSearchLine::Status)
StatusSearchLine::StatusSearchLine(QWidget *parent)
    : QLineEdit(parent)
{
    setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::BottomEdge}));
}

StatusSearchLine::~StatusSearchLine() = default;

void StatusSearchLine::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        Q_EMIT forceLostFocus();
    } else {
        QLineEdit::keyPressEvent(e);
    }
}

#include "moc_statussearchline.cpp"
