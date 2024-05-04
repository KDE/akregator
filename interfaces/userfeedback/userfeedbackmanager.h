/*
   SPDX-FileCopyrightText: 2020-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregatorinterfaces_export.h"
#include <QObject>
namespace KUserFeedback
{
class Provider;
}
namespace Akregator
{
class AKREGATORINTERFACES_EXPORT UserFeedBackManager : public QObject
{
    Q_OBJECT
public:
    explicit UserFeedBackManager(QObject *parent = nullptr);
    static UserFeedBackManager *self();

    [[nodiscard]] KUserFeedback::Provider *userFeedbackProvider() const;

private:
    KUserFeedback::Provider *const mUserFeedbackProvider;
};
}
