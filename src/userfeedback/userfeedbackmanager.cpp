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

#include "userfeedbackmanager.h"
#include <KUserFeedback/ApplicationVersionSource>
#include <KUserFeedback/PlatformInfoSource>
#include <KUserFeedback/ScreenInfoSource>
#include <KUserFeedback/QtVersionSource>
#include <KUserFeedback/Provider>
#include <KUserFeedback/StartCountSource>
#include <KUserFeedback/UsageTimeSource>
#include <KUserFeedback/LocaleInfoSource>

UserFeedBackManager::UserFeedBackManager(QObject *parent)
    : QObject(parent)
{
    mUserFeedbackProvider = new KUserFeedback::Provider(this);
    mUserFeedbackProvider->setProductIdentifier(QStringLiteral("org.kde.korganizer"));
    mUserFeedbackProvider->setFeedbackServer(QUrl(QStringLiteral("https://telemetry.kde.org/")));
    mUserFeedbackProvider->setSubmissionInterval(7);
    mUserFeedbackProvider->setApplicationStartsUntilEncouragement(5);
    mUserFeedbackProvider->setEncouragementDelay(30);

    mUserFeedbackProvider->addDataSource(new KUserFeedback::ApplicationVersionSource);
    mUserFeedbackProvider->addDataSource(new KUserFeedback::PlatformInfoSource);
    mUserFeedbackProvider->addDataSource(new KUserFeedback::ScreenInfoSource);
    mUserFeedbackProvider->addDataSource(new KUserFeedback::QtVersionSource);

    mUserFeedbackProvider->addDataSource(new KUserFeedback::StartCountSource);
    mUserFeedbackProvider->addDataSource(new KUserFeedback::UsageTimeSource);

    mUserFeedbackProvider->addDataSource(new KUserFeedback::LocaleInfoSource);
}

UserFeedBackManager *UserFeedBackManager::self()
{
    static UserFeedBackManager s_self;
    return &s_self;
}

KUserFeedback::Provider *UserFeedBackManager::userFeedbackProvider() const
{
    return mUserFeedbackProvider;
}
