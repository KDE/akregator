/*
   SPDX-FileCopyrightText: 2020-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "akregatorinterfaces_export.h"
#ifdef USE_KUSERFEEDBACK_QT6
#include <KUserFeedbackQt6/Provider>
#else
#include <KUserFeedback/Provider>
#endif
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
