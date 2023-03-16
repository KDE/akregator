/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "kcmutils_version.h"
#include <KCModule>
#include <QVariant>

#include "akregatorconfig.h"

class QButtonGroup;

class KCMAkregatorArchiveConfig : public KCModule
{
    Q_OBJECT

public:
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    explicit KCMAkregatorArchiveConfig(QWidget *parent, const QVariantList &args);
#else
    explicit KCMAkregatorArchiveConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
#endif

    void setArchiveMode(int mode);
    int archiveMode() const;

    void load() override;
    void save() override;

private:
    QWidget *const m_widget;
    QButtonGroup *m_archiveModeGroup = nullptr;
};
