/*
   SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef AKREGATORUSERFEEDBACKPROVIDER_H
#define AKREGATORUSERFEEDBACKPROVIDER_H

#include <KUserFeedback/Provider>
#include "akregatorinterfaces_export.h"
namespace Akregator {
class AKREGATORINTERFACES_EXPORT AkregatorUserFeedbackProvider : public KUserFeedback::Provider
{
    Q_OBJECT
public:
    explicit AkregatorUserFeedbackProvider(QObject *parent = nullptr);
    ~AkregatorUserFeedbackProvider();
};
}
#endif // AKREGATORUSERFEEDBACKPROVIDER_H
