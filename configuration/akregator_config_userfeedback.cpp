/*
   SPDX-FileCopyrightText: 2020-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregator_config_userfeedback.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QHBoxLayout>
#ifdef WITH_KUSERFEEDBACK
#include "userfeedback/userfeedbackmanager.h"
#include <KUserFeedback/FeedbackConfigWidget>
#endif

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorUserFeedBackConfig, "akregator_config_userfeedback.json")
KCMAkregatorUserFeedBackConfig::KCMAkregatorUserFeedBackConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , mUserFeedbackWidget(new KUserFeedback::FeedbackConfigWidget(widget()))
{
    auto lay = new QHBoxLayout(widget());
    lay->setContentsMargins({});

    connect(mUserFeedbackWidget, &KUserFeedback::FeedbackConfigWidget::configurationChanged, this, &KCMAkregatorUserFeedBackConfig::markAsChanged);

    lay->addWidget(mUserFeedbackWidget);
}

void KCMAkregatorUserFeedBackConfig::save()
{
    Akregator::UserFeedBackManager::self()->userFeedbackProvider()->setTelemetryMode(mUserFeedbackWidget->telemetryMode());
    Akregator::UserFeedBackManager::self()->userFeedbackProvider()->setSurveyInterval(mUserFeedbackWidget->surveyInterval());
    setNeedsSave(false);
}

void KCMAkregatorUserFeedBackConfig::load()
{
    mUserFeedbackWidget->setFeedbackProvider(Akregator::UserFeedBackManager::self()->userFeedbackProvider());
    setNeedsSave(false);
}

void KCMAkregatorUserFeedBackConfig::defaults()
{
    // TODO fixme default values
    mUserFeedbackWidget->setFeedbackProvider(Akregator::UserFeedBackManager::self()->userFeedbackProvider());
}

#include "akregator_config_userfeedback.moc"

#include "moc_akregator_config_userfeedback.cpp"
