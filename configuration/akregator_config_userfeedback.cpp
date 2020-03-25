/*
   Copyright (C) 2020 Laurent Montel <montel@kde.org>

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

#include "akregator_config_userfeedback.h"
#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>
#include <QHBoxLayout>
#ifdef WITH_KUSERFEEDBACK
#include <KUserFeedback/FeedbackConfigWidget>
#include "userfeedback/userfeedbackmanager.h"
#endif

using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorUserFeedBackConfigFactory, registerPlugin<KCMAkregatorUserFeedBackConfig>();
                 )

KCMAkregatorUserFeedBackConfig::KCMAkregatorUserFeedBackConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);

    KAboutData *about = new KAboutData(QStringLiteral("kcmaddressbookuserfeedbackconfig"),
                                       i18n("Configure User Feedback"),
                                       QString(), QString(), KAboutLicense::GPL,
                                       i18n("(c), 2020 Laurent Montel"));

    about->addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));

    setAboutData(about);
    mUserFeedbackWidget = new KUserFeedback::FeedbackConfigWidget(this);
    connect(mUserFeedbackWidget, &KUserFeedback::FeedbackConfigWidget::configurationChanged, this, &KCMAkregatorUserFeedBackConfig::markAsChanged);

    lay->addWidget(mUserFeedbackWidget);
    mUserFeedbackWidget->setFeedbackProvider(Akregator::UserFeedBackManager::self()->userFeedbackProvider());
}

void KCMAkregatorUserFeedBackConfig::save()
{
    Akregator::UserFeedBackManager::self()->userFeedbackProvider()->setTelemetryMode(mUserFeedbackWidget->telemetryMode());
    Akregator::UserFeedBackManager::self()->userFeedbackProvider()->setSurveyInterval(mUserFeedbackWidget->surveyInterval());
}

void KCMAkregatorUserFeedBackConfig::load()
{
    mUserFeedbackWidget->setFeedbackProvider(Akregator::UserFeedBackManager::self()->userFeedbackProvider());
}

void KCMAkregatorUserFeedBackConfig::defaults()
{
    //TODO fixme default values
    mUserFeedbackWidget->setFeedbackProvider(Akregator::UserFeedBackManager::self()->userFeedbackProvider());
}

#include "akregator_config_userfeedback.moc"
