/*
   SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
    auto *lay = new QHBoxLayout(this);
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
