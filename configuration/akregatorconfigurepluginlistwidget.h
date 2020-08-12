/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AKREGATORCONFIGUREPLUGINLISTWIDGET_H
#define AKREGATORCONFIGUREPLUGINLISTWIDGET_H

#include <PimCommon/ConfigurePluginsListWidget>

class AkregatorConfigurePluginListWidget : public PimCommon::ConfigurePluginsListWidget
{
    Q_OBJECT
public:
    explicit AkregatorConfigurePluginListWidget(QWidget *parent = nullptr);
    ~AkregatorConfigurePluginListWidget();

    void save() override;
    void doLoadFromGlobalSettings() override;
    void doResetToDefaultsOther() override;
    void initialize() override;
private:
    void slotConfigureClicked(const QString &configureGroupName, const QString &identifier);
    QVector<PluginItem *> mPluginWebEngineItems;
};

#endif // AKREGATORCONFIGUREPLUGINLISTWIDGET_H
