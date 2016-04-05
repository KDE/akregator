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

#include "akregator_config_webengineurlinterceptor.h"
#include "akregatorconfig.h"
#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QVBoxLayout>
#include <QTabWidget>

#include <MessageViewer/NetworkPluginUrlInterceptor>
#include <MessageViewer/NetworkPluginUrlInterceptorConfigureWidget>
#include <MessageViewer/NetworkUrlInterceptorPluginManager>

using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorWebEngineUrlInterceptorConfigFactory, registerPlugin<KCMAkregatorWebEngineUrlInterceptorConfig>();)

KCMAkregatorWebEngineUrlInterceptorConfig::KCMAkregatorWebEngineUrlInterceptorConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);

    KAboutData *about = new KAboutData(QStringLiteral("kcmakrwebengineurlinterceptorconfig"),
                                       i18n("Configure AdBlock"),
                                       QString(), QString(), KAboutLicense::GPL,
                                       i18n("(c), 2016 Laurent Montel"));

    about->addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));

    setAboutData(about);
    QTabWidget *tab = new QTabWidget(this);
    lay->addWidget(tab);
    Q_FOREACH(MessageViewer::NetworkPluginUrlInterceptor *plugin, MessageViewer::NetworkUrlInterceptorPluginManager::self()->pluginsList()) {
        if (plugin->hasConfigureSupport()) {
            MessageViewer::NetworkPluginUrlInterceptorConfigureWidgetSetting settings = plugin->createConfigureWidget(this);
            MessageViewer::NetworkPluginUrlInterceptorConfigureWidget *configureWidget = settings.configureWidget;
            tab->addTab(configureWidget, settings.name);
            connect(configureWidget, &MessageViewer::NetworkPluginUrlInterceptorConfigureWidget::configureChanged, this, &KCMAkregatorWebEngineUrlInterceptorConfig::slotConfigChanged);
        }
    }

    setLayout(lay);
}

void KCMAkregatorWebEngineUrlInterceptorConfig::slotConfigChanged()
{
    Q_EMIT changed(true);
}

void KCMAkregatorWebEngineUrlInterceptorConfig::save()
{
}

void KCMAkregatorWebEngineUrlInterceptorConfig::load()
{
}

void KCMAkregatorWebEngineUrlInterceptorConfig::defaults()
{
}

#include "akregator_config_webengineurlinterceptor.moc"