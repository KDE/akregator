/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once
#include <KCModule>

class QWidget;

class KCMAkregatorGeneralConfig : public KCModule
{
    Q_OBJECT

public:
    explicit KCMAkregatorGeneralConfig(QObject *parent, const KPluginMetaData &data);

private:
    QWidget *const m_widget;
};
