/*
   Copyright (C) 2016-2017 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCMAkregatorPluginsConfig_H
#define KCMAkregatorPluginsConfig_H

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
    KCMAkregatorPluginsConfig(QWidget *parent, const QVariantList &args);
    void load() override;
    void save() override;
    void defaults() override;
private Q_SLOTS:
    void slotConfigChanged();
private:
    PimCommon::ConfigurePluginsWidget *mConfigurePluginWidget = nullptr;
};
}
#endif // KCMAkregatorPluginsConfig_H
