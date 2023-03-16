/*
   SPDX-FileCopyrightText: 2020-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "akregator_config_userfeedback.h"
#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QHBoxLayout>
#ifdef WITH_KUSERFEEDBACK
#include "userfeedback/userfeedbackmanager.h"
#ifdef USE_KUSERFEEDBACK_QT6
#include <KUserFeedbackQt6/FeedbackConfigWidget>
#else
#include <KUserFeedback/FeedbackConfigWidget>
#endif
#endif

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorUserFeedBackConfig, "akregator_config_userfeedback.json")
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
KCMAkregatorUserFeedBackConfig::KCMAkregatorUserFeedBackConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
#else
KCMAkregatorUserFeedBackConfig::KCMAkregatorUserFeedBackConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KCModule(parent, data, args)
#endif
{
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    auto lay = new QHBoxLayout(this);
#else
    auto lay = new QHBoxLayout(widget());
#endif
    lay->setContentsMargins({});

#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    mUserFeedbackWidget = new KUserFeedback::FeedbackConfigWidget(this);
#else
    mUserFeedbackWidget = new KUserFeedback::FeedbackConfigWidget(widget());
#endif
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
    // TODO fixme default values
    mUserFeedbackWidget->setFeedbackProvider(Akregator::UserFeedBackManager::self()->userFeedbackProvider());
}

#include "akregator_config_userfeedback.moc"
