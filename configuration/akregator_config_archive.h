/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef AKREGATOR_CONFIG_ARCHIVE_H
#define AKREGATOR_CONFIG_ARCHIVE_H

#include <KCModule>

#include <QVariant>

#include "akregatorconfig.h"

class QButtonGroup;

class KCMAkregatorArchiveConfig : public KCModule
{
    Q_OBJECT

public:
    explicit KCMAkregatorArchiveConfig(QWidget *parent, const QVariantList &args);

    void setArchiveMode(int mode);
    int archiveMode() const;

    void load() override;
    void save() override;

private:
    QWidget *m_widget = nullptr;
    QButtonGroup *m_archiveModeGroup = nullptr;
};

#endif // AKREGATOR_CONFIG_ARCHIVE_H
