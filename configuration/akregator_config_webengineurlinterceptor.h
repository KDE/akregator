/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KCMAkregatorWebEngineUrlInterceptorConfig_H
#define KCMAkregatorWebEngineUrlInterceptorConfig_H

#include <KCModule>

#include <QVariant>
namespace MessageViewer
{
class NetworkPluginUrlInterceptorConfigureWidget;
}
namespace Akregator
{
class KCMAkregatorWebEngineUrlInterceptorConfig : public KCModule
{
    Q_OBJECT
public:
    KCMAkregatorWebEngineUrlInterceptorConfig(QWidget *parent, const QVariantList &args);
    void load() Q_DECL_OVERRIDE;
    void save() Q_DECL_OVERRIDE;
    void defaults() Q_DECL_OVERRIDE;
private Q_SLOTS:
    void slotConfigChanged();
private:
    QList<MessageViewer::NetworkPluginUrlInterceptorConfigureWidget *> mListPlugins;

};
}
#endif // KCMAkregatorWebEngineUrlInterceptorConfig_H
