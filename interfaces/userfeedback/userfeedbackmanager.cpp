/*
   SPDX-FileCopyrightText: 2020-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "userfeedbackmanager.h"
#include "akregatoruserfeedbackprovider.h"
using namespace Akregator;
UserFeedBackManager::UserFeedBackManager(QObject *parent)
    : QObject(parent)
{
    mUserFeedbackProvider = new AkregatorUserFeedbackProvider(this);
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
