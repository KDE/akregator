/*
   SPDX-FileCopyrightText: 2024 Laurent Montel <montel.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "whatsnewtranslations.h"

WhatsNewTranslations::WhatsNewTranslations() = default;

WhatsNewTranslations::~WhatsNewTranslations() = default;

// Use by newFeaturesMD5
QList<KLazyLocalizedString> WhatsNewTranslations::lastNewFeatures() const
{
    const QList<KLazyLocalizedString> info{kli18n("Add windows support")};
    return info;
}

QList<PimCommon::WhatsNewInfo> WhatsNewTranslations::createWhatsNewInfo() const
{
    QList<PimCommon::WhatsNewInfo> listInfo;
    {
        PimCommon::WhatsNewInfo info;
        const QStringList lst{i18n("Implement Plasma Activity."), i18n("Add new whatsnew widget.")};
        info.setNewFeatures(lst);
        info.setVersion(QStringLiteral("6.3.0"));
        listInfo.append(std::move(info));
    }
    {
        PimCommon::WhatsNewInfo info;
        QStringList lst;
        for (const KLazyLocalizedString &l : lastNewFeatures()) {
            lst += l.toString();
        }
        info.setNewFeatures(lst);
        info.setVersion(QStringLiteral("6.4.0"));
        listInfo.append(std::move(info));
    }
    return listInfo;
}
