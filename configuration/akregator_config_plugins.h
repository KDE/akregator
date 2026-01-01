/*
   SPDX-FileCopyrightText: 2016-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include <KCModule>
namespace PimCommon
{
class ConfigurePluginsWidget;
}

namespace Akregator
{
class KCMAkregatorPluginsConfig : public KCModule
{
    Q_OBJECT
public:
    explicit KCMAkregatorPluginsConfig(QObject *parent, const KPluginMetaData &data);
    void load() override;
    void save() override;
    void defaults() override;
private Q_SLOTS:
    void slotConfigChanged();

private:
    PimCommon::ConfigurePluginsWidget *mConfigurePluginWidget = nullptr;
};
}
