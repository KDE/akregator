/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_CONFIG_GENERAL_H
#define AKREGATOR_CONFIG_GENERAL_H

#include <KCModule>

#include <QVariant>

class QWidget;

class KCMAkregatorGeneralConfig : public KCModule
{
    Q_OBJECT

public:
    explicit KCMAkregatorGeneralConfig(QWidget *parent, const QVariantList &args);

private:
    QWidget *m_widget = nullptr;
};

#endif // AKREGATOR_CONFIG_GENERAL_H
