/*
    This file is part of Akregator.
    SPDX-FileCopyrightText: 2008 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akregator_config_appearance.h"
#include "akregatorconfig.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QAbstractSlider>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_CLASS_WITH_JSON(KCMAkregatorAppearanceConfig, "akregator_config_appearance.json")

KCMAkregatorAppearanceConfig::KCMAkregatorAppearanceConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , m_widget(new QWidget(this))
{
    m_ui.setupUi(m_widget);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_widget);

    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.kcfg_ColorUnreadArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.kcfg_ColorNewArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.lbl_newArticles, &QWidget::setEnabled);
    connect(m_ui.kcfg_UseCustomColors, &QAbstractButton::toggled, m_ui.lbl_unreadArticles, &QWidget::setEnabled);

    connect(m_ui.slider_minimumFontSize, &QAbstractSlider::valueChanged, m_ui.kcfg_MinimumFontSize, &QSpinBox::setValue);
    connect(m_ui.slider_mediumFontSize, &QAbstractSlider::valueChanged, m_ui.kcfg_MediumFontSize, &QSpinBox::setValue);

    connect(m_ui.slider_minimumFontSize, &QAbstractSlider::sliderMoved, m_ui.kcfg_MinimumFontSize, &QSpinBox::setValue);
    connect(m_ui.slider_mediumFontSize, &QAbstractSlider::sliderMoved, m_ui.kcfg_MediumFontSize, &QSpinBox::setValue);

    connect(m_ui.kcfg_MinimumFontSize, qOverload<int>(&KPluralHandlingSpinBox::valueChanged), m_ui.slider_minimumFontSize, &QSlider::setValue);
    connect(m_ui.kcfg_MediumFontSize, qOverload<int>(&KPluralHandlingSpinBox::valueChanged), m_ui.slider_mediumFontSize, &QSlider::setValue);
    auto about = new KAboutData(QStringLiteral("kcmakrappearanceconfig"),
                                i18n("Configure Feed Reader Appearance"),
                                QString(),
                                QString(),
                                KAboutLicense::GPL,
                                i18n("(c), 2004 - 2008 Frank Osterfeld"));

    about->addAuthor(i18n("Frank Osterfeld"), QString(), QStringLiteral("osterfeld@kde.org"));

    setAboutData(about);

    m_ui.slider_minimumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MinimumFontSize")));
    m_ui.slider_mediumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MediumFontSize")));
    m_ui.lbl_MinimumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MinimumFontSize")));
    m_ui.lbl_MediumFontSize->setDisabled(Settings::self()->isImmutable(QStringLiteral("MediumFontSize")));

    addConfig(Settings::self(), m_widget);
}

#include "akregator_config_appearance.moc"
