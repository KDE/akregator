/*
   SPDX-FileCopyrightText: 2020-2022 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregatorinterfaces_export.h"
#include <KUserFeedback/Provider>
namespace Akregator
{
class AKREGATORINTERFACES_EXPORT AkregatorUserFeedbackProvider : public KUserFeedback::Provider
{
    Q_OBJECT
public:
    explicit AkregatorUserFeedbackProvider(QObject *parent = nullptr);
    ~AkregatorUserFeedbackProvider() override;
};
}
