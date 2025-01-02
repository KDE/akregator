/*
   SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <PimCommon/WhatsNewTranslationsBase>

class WhatsNewTranslations : public PimCommon::WhatsNewTranslationsBase
{
public:
    WhatsNewTranslations();
    ~WhatsNewTranslations() override;

    [[nodiscard]] QList<PimCommon::WhatsNewInfo> createWhatsNewInfo() const override;
    [[nodiscard]] QList<KLazyLocalizedString> lastNewFeatures() const override;
};
