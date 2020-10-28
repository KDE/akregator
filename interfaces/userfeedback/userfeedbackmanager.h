/*
   SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef USERFEEDBACKMANAGER_H
#define USERFEEDBACKMANAGER_H

#include <QObject>
#include "akregatorinterfaces_export.h"
namespace KUserFeedback {
class Provider;
}
namespace Akregator {
class AKREGATORINTERFACES_EXPORT UserFeedBackManager : public QObject
{
    Q_OBJECT
public:
    explicit UserFeedBackManager(QObject *parent = nullptr);
    static UserFeedBackManager *self();

    KUserFeedback::Provider *userFeedbackProvider() const;

private:
    KUserFeedback::Provider *mUserFeedbackProvider = nullptr;
};
}
#endif // USERFEEDBACKMANAGER_H
