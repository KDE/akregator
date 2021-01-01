/*
   SPDX-FileCopyrightText: 2020-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCM_Akregator_Config_Plugins_H
#define KCM_Akregator_Config_Plugins_H

#include <KCModule>

#include <QVariant>
namespace KUserFeedback {
class FeedbackConfigWidget;
}

namespace Akregator {
class KCMAkregatorUserFeedBackConfig : public KCModule
{
    Q_OBJECT
public:
    KCMAkregatorUserFeedBackConfig(QWidget *parent, const QVariantList &args);
    void load() override;
    void save() override;
    void defaults() override;
private:
    KUserFeedback::FeedbackConfigWidget *mUserFeedbackWidget = nullptr;
};
}
#endif // KCM_Akregator_Config_Plugins_H
