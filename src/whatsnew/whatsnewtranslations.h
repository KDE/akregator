/*
   SPDX-FileCopyrightText: 2024-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "config-akregator.h"
#include <TextAddonsWidgets/WhatsNewTranslationsBase>
class WhatsNewTranslations : public TextAddonsWidgets::WhatsNewTranslationsBase
{
public:
    WhatsNewTranslations();
    ~WhatsNewTranslations() override;

    [[nodiscard]] QList<TextAddonsWidgets::WhatsNewInfo> createWhatsNewInfo() const override;
    [[nodiscard]] QList<KLazyLocalizedString> lastNewFeatures() const override;
};
