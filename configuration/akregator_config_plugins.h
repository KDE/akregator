/*
   SPDX-FileCopyrightText: 2016-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AKREGATOR_CONFIG_PLUGINS_H
#define AKREGATOR_CONFIG_PLUGINS_H

#include <KCModule>
#include <QVariant>
namespace PimCommon {
class ConfigurePluginsWidget;
}

namespace Akregator {
class KCMAkregatorPluginsConfig : public KCModule
{
    Q_OBJECT
public:
    explicit KCMAkregatorPluginsConfig(QWidget *parent, const QVariantList &args);
    void load() override;
    void save() override;
    void defaults() override;
private Q_SLOTS:
    void slotConfigChanged();
private:
    PimCommon::ConfigurePluginsWidget *mConfigurePluginWidget = nullptr;
};
}
#endif //  AKREGATOR_CONFIG_PLUGINS_H
