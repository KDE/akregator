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

#include "akregator_config_adblock.h"
#include "akregatorconfig.h"
#include "messageviewer/messageviewersettings.h"
#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>

#include <QVBoxLayout>

#include <MessageViewer/AdBlockSettingWidget>

using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorAdBlockConfigFactory, registerPlugin<KCMAkregatorAdBlockConfig>();)

KCMAkregatorAdBlockConfig::KCMAkregatorAdBlockConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);

    KAboutData *about = new KAboutData(QStringLiteral("kcmakrablockconfig"),
                                       i18n("Configure AdBlock"),
                                       QString(), QString(), KAboutLicense::GPL,
                                       i18n("(c), 2016 Laurent Montel"));

    about->addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));

    setAboutData(about);

    mWidget = new MessageViewer::AdBlockSettingWidget;
    lay->addWidget(mWidget);
    connect(mWidget, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    setLayout(lay);
}

void KCMAkregatorAdBlockConfig::save()
{
    mWidget->save();
    MessageViewer::MessageViewerSettings::self()->save();
}

void KCMAkregatorAdBlockConfig::load()
{
    mWidget->doLoadFromGlobalSettings();
}

void KCMAkregatorAdBlockConfig::defaults()
{
    mWidget->doResetToDefaultsOther();
}

#include "akregator_config_adblock.moc"
