/*
   SPDX-FileCopyrightText: 2016-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <PimCommon/ConfigurePluginsListWidget>

class AkregatorConfigurePluginListWidget : public PimCommon::ConfigurePluginsListWidget
{
    Q_OBJECT
public:
    explicit AkregatorConfigurePluginListWidget(QWidget *parent = nullptr);
    ~AkregatorConfigurePluginListWidget() override;

    void save() override;
    void doLoadFromGlobalSettings() override;
    void doResetToDefaultsOther() override;
    void initialize() override;

private:
    void slotConfigureClicked(const QString &configureGroupName, const QString &identifier);
    QList<PluginItem *> mPluginWebEngineItems;
};
