/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "command.h"

#include <QEventLoop>
#include <QPointer>
#include <QWidget>

using namespace Akregator;

class Akregator::CommandPrivate
{
public:
    CommandPrivate();
    QPointer<QWidget> parentWidget;
};

CommandPrivate::CommandPrivate()
    : parentWidget()
{
}

Command::Command(QObject *parent)
    : QObject(parent)
    , d(new CommandPrivate)
{
}

Command::~Command() = default;

QWidget *Command::parentWidget() const
{
    return d->parentWidget;
}

void Command::setParentWidget(QWidget *parentWidget)
{
    d->parentWidget = parentWidget;
}

void Command::start()
{
    doStart();
    Q_EMIT started();
}

void Command::abort()
{
    doAbort();
}

void Command::done()
{
    Q_EMIT finished();
    deleteLater();
}

void Command::waitForFinished()
{
    QEventLoop loop;
    connect(this, &Command::finished, &loop, &QEventLoop::quit);
    loop.exec();
}

#include "moc_command.cpp"
