/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <KCModule>
class QButtonGroup;

class KCMAkregatorArchiveConfig : public KCModule
{
    Q_OBJECT

public:
    explicit KCMAkregatorArchiveConfig(QObject *parent, const KPluginMetaData &data);

    void setArchiveMode(int mode);
    int archiveMode() const;

    void load() override;
    void save() override;

private:
    QWidget *const m_widget;
    QButtonGroup *const m_archiveModeGroup;
};
