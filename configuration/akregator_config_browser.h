/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KCModule>

#include <QVariant>

class QWidget;

class KCMAkregatorBrowserConfig : public KCModule
{
    Q_OBJECT

public:
    explicit KCMAkregatorBrowserConfig(QWidget *parent, const QVariantList &args);

private:
    QWidget *const m_widget;
};
