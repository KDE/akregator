/*
   SPDX-FileCopyrightText: 2020-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include <KCModule>

namespace KUserFeedback
{
class FeedbackConfigWidget;
}

namespace Akregator
{
class KCMAkregatorUserFeedBackConfig : public KCModule
{
    Q_OBJECT
public:
    explicit KCMAkregatorUserFeedBackConfig(QObject *parent, const KPluginMetaData &data);
    void load() override;
    void save() override;
    void defaults() override;

private:
    KUserFeedback::FeedbackConfigWidget *const mUserFeedbackWidget;
};
}
