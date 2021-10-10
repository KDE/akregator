/*
    This file is part of Akregator.

    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akregatorinterfaces_export.h"

#include <QObject>

#include <memory>

class QWidget;

namespace Akregator
{
class CommandPrivate;

class AKREGATORINTERFACES_EXPORT Command : public QObject
{
    Q_OBJECT
public:
    explicit Command(QObject *parent = nullptr);
    ~Command() override;

    QWidget *parentWidget() const;
    void setParentWidget(QWidget *parentWidget);

    void start();
    void abort();

    void waitForFinished();

Q_SIGNALS:
    void started();
    void finished();
    void progress(int percent, const QString &msg);

protected:
    virtual void doStart() = 0;
    virtual void doAbort() = 0;

    void done();

private:
    std::unique_ptr<CommandPrivate> const d;
};
}

